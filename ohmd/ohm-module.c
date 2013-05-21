/*
 * Copyright (C) 2007 Richard Hughes <richard@hughsie.com>
 *
 * Licensed under the GNU Lesser General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>

#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#include <dirent.h>
#include <sys/stat.h>

#include <glib/gi18n.h>
#include <gmodule.h>

#include "ohm-debug.h"
#include "ohm-module.h"
#include "ohm-plugin-internal.h"
#include "ohm-conf.h"

#define OHM_MODULE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), OHM_TYPE_MODULE, OhmModulePrivate))

struct OhmModulePrivate
{
	GSList			*mod_require;
	GSList			*mod_suggest;
	GSList			*mod_prevent;
	GSList			*mod_loaded;	/* list of loaded module names */
	GSList			*plugins;	/* list of loaded OhmPlugin's */
	GHashTable		*interested;
	OhmConf			*conf;
	gboolean		 do_extra_checks;
	gchar			**modules_banned;
	gchar			**modules_suggested;
	gchar			**modules_required;
};

/* used as a hash entry type to provide int-passing services to the plugin */
typedef struct {
	OhmPlugin		*plugin;
	gint			 id;
} OhmModuleNotify;

G_DEFINE_TYPE (OhmModule, ohm_module, G_TYPE_OBJECT)


static GHashTable *symtable;


static gboolean
free_notify_list (const gchar *key, GSList *list, gpointer userdata)
{
	GSList *l;

	for (l=list; l != NULL; l=l->next) {
		g_slice_free (OhmModuleNotify, l->data);
	}
	g_slist_free (list);

	return TRUE;
}

static void
key_changed_cb (OhmConf     *conf,
		const gchar *key,
		gint	     value,
		OhmModule   *module)
{
	GSList *entry;
	GSList *l;
	OhmModuleNotify *notif;
	const gchar *name;

	ohm_debug ("key changed! %s : %i", key, value);

	/* if present, add to SList, if not, add to hash as slist object */
	entry = g_hash_table_lookup (module->priv->interested, key);

	/* a key has changed that none of the plugins are watching */
	if (entry == NULL) {
		return;
	}

	ohm_debug ("found watched key %s", key);
	/* go thru the SList and notify each plugin */
	for (l=entry; l != NULL; l=l->next) {
		notif = (OhmModuleNotify *) l->data;
		name = ohm_plugin_get_name (notif->plugin);
		ohm_debug ("notify %s with id:%i", name, notif->id);
		ohm_plugin_notify (notif->plugin, key, notif->id, value);
	}
}

static void
add_interesteds (OhmModule   *module, OhmPlugin   *plugin)
{
	GSList *entry;
	OhmModuleNotify *notif;
	const OhmPluginKeyIdMap *interested;
	
	if (plugin->interested == NULL)
		return;

	for (interested = plugin->interested; interested->key_name; interested++) {
		ohm_debug ("add interested! %s : %i", interested->key_name, interested->local_key_id);

		/* if present, add to SList, if not, add to hash as slist object */
		entry = g_hash_table_lookup (module->priv->interested, interested->key_name);

		/* create a new notifier, and copy over the data */
		notif = g_slice_new (OhmModuleNotify);
		notif->plugin = plugin;
		notif->id = interested->local_key_id;

		entry = g_slist_prepend (entry, (gpointer) notif);
		g_hash_table_insert (module->priv->interested, (gpointer) interested->key_name, entry);
	}
}


static gboolean
add_provides (OhmModule *module, OhmPlugin *plugin)
{
	GError *error;
	const char **provides = plugin->provides;
	gboolean ret=TRUE;
	error = NULL;

	if (provides == NULL)
		return TRUE;

	for (; *provides; provides++) {
		ohm_debug ("%s provides %s", ohm_plugin_get_name(plugin), *provides);
		/* provides keys are never public and are always preset at zero */
		ret &= ohm_conf_add_key (module->priv->conf, *provides, 0, FALSE, &error);
		if (ret == FALSE) {
			ohm_debug ("Cannot provide key %s: %s", *provides, error->message);
			g_error_free (error);
		}
	}
	return ret;
}

static void
add_names (GSList **l, const char **names)
{
	if (names == NULL)
		return;

	for (;*names; names++) {
		*l = g_slist_prepend (*l, (gpointer) *names);
	}
}

static gboolean
ohm_module_add_plugin (OhmModule *module, const gchar *name)
{
	OhmPlugin *plugin;

	/* setup new plugin */
	plugin = ohm_plugin_new ();

	/* try to load plugin, this might fail */
	if (!ohm_plugin_load (plugin, name))
		return FALSE;

	ohm_debug ("adding %s to module list", name);
	module->priv->plugins = g_slist_prepend (module->priv->plugins, (gpointer) plugin);
	add_names (&module->priv->mod_require, plugin->requires);
	add_names (&module->priv->mod_suggest, plugin->suggests);
	add_names (&module->priv->mod_prevent, plugin->prevents);
	add_interesteds (module, plugin);

	if (!add_provides (module, plugin))
		return FALSE;
	else
		return TRUE;
}

/* adds plugins from require and suggests lists. Failure of require is error, failure of suggests is warning */
/* we have to make sure we do not load banned plugins from the prevent list or load already loaded plugins */
/* this should be very fast (two or three runs) for the common case */
static void
ohm_module_add_all_plugins (OhmModule *module)
{
	GSList *lfound;
	GSList *current;
	gchar *entry;
	gboolean ret;

	/* go through requires */
	if (module->priv->mod_require != NULL) {
		ohm_debug ("processing require");
	}
	while (module->priv->mod_require != NULL) {
		current = module->priv->mod_require;
		entry = (gchar *) current->data;

		/* make sure it's not banned */
		lfound = g_slist_find_custom (module->priv->mod_prevent, entry, (GCompareFunc) strcmp);
		if (lfound != NULL) {
			g_error ("module listed in require is also listed in prevent");
		}

		/* make sure it's not already loaded */
		lfound = g_slist_find_custom (module->priv->mod_loaded, entry, (GCompareFunc) strcmp);
		if (lfound == NULL) {
			/* load module */
			ret = ohm_module_add_plugin (module, entry);
			if (ret == FALSE) {
				char buf[64];

				/* Work around the core dump abort problem associated
				 * with g_error() */

				snprintf (buf, 64,
						"module %s failed to load but listed in require", entry);
				g_error ("%s", buf);
			}

			/* add to loaded list */
			module->priv->mod_loaded = g_slist_prepend (module->priv->mod_loaded, (gpointer) entry);
		} else {
			ohm_debug ("module %s already loaded", entry);
		}

		/* remove this entry from the list, and use cached current as the head may have changed */
		module->priv->mod_require = g_slist_delete_link (module->priv->mod_require, current);
	}

	/* go through suggest */
	if (module->priv->mod_suggest != NULL) {
		ohm_debug ("processing suggest");
	}
	while (module->priv->mod_suggest != NULL) {
		current = module->priv->mod_suggest;
		entry = (gchar *) current->data;

		/* make sure it's not banned */
		lfound = g_slist_find_custom (module->priv->mod_prevent, entry, (GCompareFunc) strcmp);
		if (lfound != NULL) {
			ohm_debug ("module %s listed in suggest is also listed in prevent, so ignoring", entry);
		} else {
			/* make sure it's not already loaded */
			lfound = g_slist_find_custom (module->priv->mod_loaded, entry, (GCompareFunc) strcmp);
			if (lfound == NULL) {
				ohm_debug ("try add: %s", entry);
				/* load module */
				ret = ohm_module_add_plugin (module, entry);
				if (ret == TRUE) {
					/* add to loaded list */
					module->priv->mod_loaded = g_slist_prepend (module->priv->mod_loaded, (gpointer) entry);
				} else {
					ohm_debug ("module %s failed to load but only suggested so no problem", entry);
				}
			}
		}
	
		/* remove this entry from the list */
		module->priv->mod_suggest = g_slist_delete_link (module->priv->mod_suggest, current);
	}
}



static gint
has_name(gconstpointer p, gconstpointer n)
{
        OhmPlugin *plugin = (OhmPlugin *)p;
        char      *name   = (char *)n;

        return strcmp(ohm_plugin_get_name(plugin), name);
}


static gboolean
ohm_module_reorder_plugins(OhmModule *module)
{
    
  /*
   * Notes:
   *
   *   We create a dependency graph of our plugins. Importing a method
   *   from a plugin adds a dependency from the importer to the importee.
   *   Declaring explicit dependency on a plugin also adds a dependency
   *   from the declaring plugin to the declared dependency.
   * 
   *   We then sort our dependency graph toplogically to determine a possible
   *   initialization order. Our topological sort algorithm is the following:
   *
   *       L <- empty list where we put the sorted elements
   *       Q <- set of all nodes with no incoming edges
   *       while Q is non-empty do
   *           remove a node n from Q
   *           insert n into L
   *           for each node m with an edge e from n to m do
   *               remove edge e from the graph
   *               if m has no other incoming edges then
   *                   insert m into Q
   *       if graph has edges then
   *           output error message (graph has a cycle)
   *       else 
   *           return topologically sorted order: L
   *
   *   In addition to L and Q we use C and P to keep track of the number of
   *   incoming edges (ie. dependencies on) of plugins and dependencies
   *   between plugins.
   */
  
  enum {
    DBG_NONE  = 0x0,
    DBG_Q     = 0x1,
    DBG_SORT  = 0x2,
    DBG_GRAPH = 0x4,
  };

  int debug_on = DBG_NONE;
  
#define DEBUG(flag, fmt, args...) do {			\
    if ((flag) & debug_on)				\
      printf("*** D: "fmt"\n" , ## args);		\
  } while (0)


#define PUSH(q, item) do {                                    \
        int __t = t##q;                                       \
        int __size = nplugin;				      \
                                                              \
        DEBUG(DBG_Q, "PUSH(%s, %s), as item #%d...", #q,      \
	      ohm_plugin_get_name(plugins[item]), __t);	      \
        q[__t++]  =   item;                                   \
        __t      %= __size;                                   \
        t##q = __t;                                           \
    } while (0)
    
            

#define POP(q) ({							\
      int         __h = h##q, __t = t##q;				\
      int         __size = nplugin;					\
      int         __item = -1;						\
      const char *__n;							\
      									\
      if (__h != __t) {							\
	__item = q[__h++];						\
	__h %= __size;							\
      }									\
      									\
      __n = (__item < 0) ? "none":ohm_plugin_get_name(plugins[__item]); \
      DEBUG(DBG_Q, "POP(%s): %s, head is #%d...", #q, __n, __h);	\
									\
      h##q = __h;							\
      __item;								\
    })


#define NEDGE(id) (C[(id)])

#define PREREQ(idx1, idx2) (P[((idx1) * nplugin) + (idx2)])

#define PLUGIN_IDX(plugin) ({					\
    int _i, _idx;						\
    								\
    for (_i = 0, _idx = -1; _idx < 0 && _i < nplugin; _i++)	\
      if (plugins[_i] == (plugin))				\
	_idx = _i;						\
								\
    if (_idx < 0)						\
      g_error("Failed to find plugin index for %s.",		\
	      ohm_plugin_get_name(plugin));			\
    								\
    _idx;							\
  })


  int *L, *Q, *C, *P;
  int  hL, hQ, tL, tQ;
  int  node;
  
  int            i, j, n, nplugin;
  GSList        *l, *reordered;
  OhmPlugin    **plugins;
  OhmPlugin     *plugin, *plg;
  ohm_method_t  *method;
  const char   **req;
  
  nplugin = 0;
  for (l = module->priv->plugins; l != NULL; l = l->next)
    nplugin++;
  
  DEBUG(DBG_GRAPH, "found %d plugins to sort", nplugin);
  
  plugins = g_new0(OhmPlugin *, nplugin);
  if (plugins == NULL)
    g_error("Failed to allocate plugin table for %d plugins.", nplugin);
  
  for (i = 0, l = module->priv->plugins; l != NULL; l = l->next, i++) {
    plugin = (OhmPlugin *)l->data;
    plugins[i] = plugin;
  }
  
  L = Q = C = P = NULL;
  n = nplugin;
  
  if ((L = malloc((n+1) * sizeof(*L))) == NULL ||
      (Q = malloc( n    * sizeof(*Q))) == NULL ||
      (C = malloc( n    * sizeof(*C))) == NULL ||
      (P = malloc( n*n  * sizeof(*P))) == NULL)
    goto fail;
  
  memset(L, -1, (n+1) * sizeof(*L));
  memset(Q, -1,  n    * sizeof(*Q));
  memset(C,  0,  n    * sizeof(*C));
  memset(P,  0,  n*n  * sizeof(*P));
  
  hL = tL = hQ = tQ = 0;
  
  /* build prerequisites (dependencies) */
  DEBUG(DBG_GRAPH, "plugin dependencies:");
  for (i = 0; i < nplugin; i++) {
    plugin = plugins[i];
    
    /* add method-importing prerequisites */
    for (method = ohm_plugin_imports(plugin); method && method->ptr; method++) {
      j = PLUGIN_IDX(method->plugin);
      PREREQ(i, j) = 1;
      DEBUG(DBG_GRAPH, "  %s depends on %s (method-import)",
	    ohm_plugin_get_name(plugin), ohm_plugin_get_name(method->plugin));
    }
    
    /* add explicitly declared prerequisites */
    for (req = plugin->requires; req && *req; req++) {
      l = g_slist_find_custom(module->priv->plugins, *req, has_name);
      if (l == NULL || (plg = (OhmPlugin *)l->data) == NULL) {
	g_error("Failed to find plugin %s.", *req);
	goto fail;
      }
      
      j = PLUGIN_IDX(plg);
      PREREQ(i, j) = 1;
      DEBUG(DBG_GRAPH, "  %s depends on %s (explicitly declared)",
	    ohm_plugin_get_name(plugin), ohm_plugin_get_name(plg));
    }
  }
  
  /* count incoming edges */
  DEBUG(DBG_GRAPH, "graph edges:");
  for (i = 0; i < nplugin; i++) {
    for (j = 0; j < nplugin; j++) {
      if (PREREQ(j, i)) {
	DEBUG(DBG_GRAPH, "  %s -> %s", ohm_plugin_get_name(plugins[i]),
	      ohm_plugin_get_name(plugins[j]));
	if (NEDGE(j) < 0)
	  NEDGE(j)  = 1;
	else
	  NEDGE(j) += 1;
      }
    }
  }
  
  /* initialize sorting: push plugins with no edges */
  DEBUG(DBG_GRAPH, "incoming edge counts:");
  for (i = 0; i < nplugin; i++) {
    DEBUG(DBG_GRAPH, "  C[%s] = %d", ohm_plugin_get_name(plugins[i]), NEDGE(i));
    if (NEDGE(i) <= 0)
      PUSH(Q, i);
  }
    
  /* sort the graph topologically */
  hQ = hL = 0;
  while ((node = POP(Q)) >= 0) {
    DEBUG(DBG_SORT, "popped %s (%d)", ohm_plugin_get_name(plugins[node]), node);
    
    PUSH(L, node);
    
    for (i = 0; i < nplugin; i++) {
      if (PREREQ(i, node)) {
	DEBUG(DBG_SORT, "  delete edge %s -> %s",
	      ohm_plugin_get_name(plugins[node]),
	      ohm_plugin_get_name(plugins[i]));
	PREREQ(i, node) = 0;
	NEDGE(i) -= 1;
	
	if (NEDGE(i) == 0) {
	  DEBUG(DBG_SORT, "  push %s (%d)", ohm_plugin_get_name(plugins[i]), i);
	  PUSH(Q, i);
	}
      }
    }
  }
  
  /* check that we have exhausted the graph */
  for (i = 0; i < nplugin; i++) {
    if (NEDGE(i) > 0) {
      g_error("Cyclical dependency among plugins (%s?).",
	      ohm_plugin_get_name(plugins[i]));
      goto fail;
    }
  }
  
  /* update plugin initialization order */
  reordered = NULL;
  for (i = 0; i < nplugin; i++) {
    if (L[i] < 0) {
      g_error("invalid plugin as #%d in reordered plugin list", i);
      goto fail;
    }
    
    ohm_debug("plugin initialized as #%d: %s", i,
	      ohm_plugin_get_name(plugins[L[i]]));
    
    reordered = g_slist_append(reordered, plugins[L[i]]);
  }
  
  g_slist_free(module->priv->plugins);
  module->priv->plugins = reordered;
  
  free(L);
  free(Q);
  free(C);
  free(P);
  return TRUE;
  
 fail:
  if (L)
    free(L);
  if (Q)
    free(Q);
  if (C)
    free(C);
  if (P)
    free(P);
  
  return FALSE;
}



static gchar **
discover_plugins(OhmModule *module, gsize *nplugin)
{
#define PLUGIN_PREFIX "libohm_"
#define PLUGIN_SUFFIX "so"
#define IS_PLUGIN_NAME(name, suff)					\
  (!strncmp(name, PLUGIN_PREFIX, sizeof(PLUGIN_PREFIX) - 1) &&		\
   !strcmp(suff, PLUGIN_SUFFIX))

#define IS_BANNED(module, name) \
  (g_slist_find_custom(module->priv->mod_prevent, name, (GCompareFunc)strcmp))

  const char *sep  = G_DIR_SEPARATOR_S;
  const char *base = getenv("OHM_PLUGIN_DIR");

  char            path[PATH_MAX], plugindir[PATH_MAX];
  char           *name, *suff;
  DIR            *dir;
  struct dirent  *entry;
  struct stat     st;

  gchar         **plugins;
  gsize           n;

  
  ohm_debug("discovering plugins...");
  
  plugins = NULL;
  n       = 0;

  if (base == NULL) {
    snprintf(plugindir, sizeof(plugindir), "%s%s%s", LIBDIR, sep, "ohm");
    base = plugindir;
  }

  if ((dir = opendir(base)) == NULL) {
    *nplugin = 0;
    return NULL;
  }
  
  while ((entry = readdir(dir)) != NULL) {
    name = entry->d_name;
    if ((suff = strrchr(name, '.')) == NULL) {
      ohm_debug("skipping non-plugin %s...", name);
      continue;
    }
    *suff++ = '\0';

    if (!IS_PLUGIN_NAME(name, suff)) {
      ohm_debug("skipping non-plugin %s...", name);
      continue;
    }
    
    snprintf(path, sizeof(path), "%s%s%s.%s", base, sep, name, PLUGIN_SUFFIX);
    if (stat(path, &st) || !S_ISREG(st.st_mode)) {
      ohm_debug("skipping non-plugin %s...", name);
      continue;
    }
    
    name += sizeof(PLUGIN_PREFIX) - 1;

    if (IS_BANNED(module, name)) {
      ohm_debug("skipping banned plugin %s...", name);
      continue;
    }

    ohm_debug("discovered %s", name);

    if ((plugins = realloc(plugins, (n + 1) * sizeof(plugins[0]))) == NULL ||
	(plugins[n] = strdup(name)) == NULL)
      goto fail;
    
    n++;
  }
  

  if ((plugins = realloc(plugins, (n + 1) * sizeof(plugins[0]))) == NULL)
      goto fail;

  closedir(dir);
  
  plugins[n] = NULL;
  *nplugin   = n;
  return plugins;
  
 fail:
  closedir(dir);
  if (plugins != NULL) {
    for (n = 0; plugins[n] != NULL; n++)
      free(plugins[n]);
    free(plugins);
  }

  *nplugin = 0;
  return NULL;
}


/**
 * ohm_module_read_defaults:
 **/
static void
ohm_module_read_defaults (OhmModule *module)
{
	GKeyFile *keyfile;
	gchar *filename;
	gchar *conf_dir;
	gsize length;
	guint i;
	GError *error;
	gboolean ret;

	/* use g_key_file. It's quick, and portable */
	keyfile = g_key_file_new ();

	/* generate path for conf file */
	conf_dir = getenv ("OHM_CONF_DIR");
	if (conf_dir != NULL) {
		/* we have from the environment */
		filename = g_build_path (G_DIR_SEPARATOR_S, conf_dir, "modules.ini", NULL);
	} else {
		/* we are running as normal */
		filename = g_build_path (G_DIR_SEPARATOR_S, SYSCONFDIR, "ohm", "modules.ini", NULL);
	}
	ohm_debug ("keyfile = %s", filename);

	/* we can never save the file back unless we remove G_KEY_FILE_NONE */
	error = NULL;
	ret = g_key_file_load_from_file (keyfile, filename, G_KEY_FILE_NONE, &error);
	if (ret == FALSE) {
		g_error ("cannot load keyfile %s", filename);
	}
	g_free (filename);

	error = NULL;
	module->priv->do_extra_checks = g_key_file_get_boolean (keyfile, "Modules", "PerformExtraChecks", &error);
	if (error != NULL) {
		ohm_debug ("PerformExtraChecks read error: %s", error->message);
		g_error_free (error);
	}
	ohm_debug ("PerformExtraChecks=%i", module->priv->do_extra_checks);

	/* read and process ModulesBanned */
	error = NULL;
	module->priv->modules_banned = g_key_file_get_string_list (keyfile, "Modules", "ModulesBanned", &length, &error);
	if (error != NULL) {
		ohm_debug ("ModulesBanned read error: %s", error->message);
		g_error_free (error);
	}
	for (i=0; i<length; i++) {
		ohm_debug ("ModulesBanned: %s", module->priv->modules_banned[i]);
		module->priv->mod_prevent = g_slist_prepend (module->priv->mod_prevent, (gpointer) module->priv->modules_banned[i]);
	}

	/* read and process ModulesSuggested */
	error = NULL;
	module->priv->modules_suggested = g_key_file_get_string_list (keyfile, "Modules", "ModulesSuggested", &length, &error);
	if (error != NULL) {
		ohm_debug ("ModulesSuggested read error: %s", error->message);
		g_error_free (error);
	}
	for (i=0; i<length; i++) {
		ohm_debug ("ModulesSuggested: %s", module->priv->modules_suggested[i]);
		module->priv->mod_suggest = g_slist_prepend (module->priv->mod_suggest, (gpointer) module->priv->modules_suggested[i]);
	}

	/* read and process ModulesRequired */
	error = NULL;
	module->priv->modules_required = g_key_file_get_string_list (keyfile, "Modules", "ModulesRequired", &length, &error);
	if (error != NULL) {
		ohm_debug ("ModulesRequired read error: %s", error->message);
		g_error_free (error);
	}
	if (length == 1 && !strcmp(module->priv->modules_required[0], "*")) {
	  g_strfreev(module->priv->modules_required);
	  module->priv->modules_required = discover_plugins(module, &length);
	}
	for (i=0; i<length; i++) {
	        ohm_debug ("ModulesRequired: %s", module->priv->modules_required[i]);
		module->priv->mod_require = g_slist_prepend (module->priv->mod_require, (gpointer) module->priv->modules_required[i]);
	}

	g_key_file_free (keyfile);
}



static char *
normalize_signature(const char *src, const char *dst)
{


  /*
   * LEVEL 5 SPAGHETTI WARNING...
   */



  static const char *qualifiers[] = {
    "const " , "volatile " , "static " , "signed " , "unsigned " ,
    "const\t", "volatile\t", "static\t", "signed\t", "unsigned\t",
    "const*" , "volatile*" , "static*" , "signed*" , "unsigned*" ,
    NULL
  };

  enum {
    NO    = 0,
    YES   = 1,
    MAYBE = 2,
  };

#define SKIP_QUALIFIERS(p) ({			\
      int i, l, __skip = 0;			\
      for (i = 0; qualifiers[i]; i++) {		\
	l = strlen(qualifiers[i]);		\
	if (!strncmp((p), qualifiers[i], l)) {	\
	  (p) += l - 1;				\
	  __skip = 1;				\
	  break;				\
	}					\
      }						\
      __skip;					\
    })
  
  int   space, pointer, comma, brace;
  char *s, *d;

  s = (char *)src;
  d = (char *)dst;

  space   = YES;
  pointer = brace = comma = 0;
  while (*s) {
    switch (*s) {
    case ' ':
    case '\t':
      if (space)
	s++;
      else {
	space = MAYBE;
	s++;
      }
      break;
      
    case '(':
    case ')':
      space = comma = NO;
      brace = (*s == '(');
      *d++  = *s++;
      break;

    case '*':
      if (!pointer && !brace)
	*d++ = ' ';
      pointer = 1;
      space = comma = brace = NO;
      *d++  = *s++;
      break;

    case ',':
      space = brace = NO;
      comma = 1;
      *d++  = *s++;
      break;

    default:
      if (space || brace || comma) {
	if (!SKIP_QUALIFIERS(s)) {
	  if (space == MAYBE && !pointer && !comma && !brace)
	    *d++ = ' ';
	  space = comma = brace = pointer = 0;
	}
      }
      else {
	*d++ = *s++;
	space = comma = brace = pointer = 0;
      }
    }
  }
  
  *d = '\0';
  return (char *)dst;
}


static gboolean
ohm_check_method_signature(const char *required, const char *provided)
{
#define SKIP_WHITESPACE(p) ({			\
      int __space = 0;				\
      while (*p == ' ' || *p == '\t')	{	\
	p++;					\
	__space = 1;				\
      }						\
      __space;					\
    })

#define NAME_START(c)				\
  (('a' <= (c) && (c) <= 'z') ||		\
   ('A' <= (c) && (c) <= 'Z') || (c) == '_')

#define NAME_CHAR(c)				\
  (('a' <= (c) && (c) <= 'z') ||		\
   ('A' <= (c) && (c) <= 'Z') ||		\
   ('0' <= (c) && (c) <= '0') || (c) == '_')
  
#define SKIP_NAME(p) do {			\
    SKIP_WHITESPACE(p);				\
    if (('0' <= (*(p)) && *(p) <= '9'))		\
      break;					\
    while (*(p) && NAME_CHAR(*(p)))		\
      (p)++;					\
  } while (0)


  char  rbuf[256], pbuf[256], *r, *p, *ra, *pa;
  int   n;
  
  /* NULL means skip checks */
  if (required == NULL || provided == NULL)
    return TRUE;

  /* normalize signatures for easier comparison */
  r = ra = normalize_signature(required, rbuf);
  p = pa = normalize_signature(provided, pbuf);

  /* find the beginning of each argument list */
  while (*ra != '\0')
    ra++;
  while (*ra != ')' && ra > r)
    ra--;
  if (*ra == ')') {
    n = 1;
    while (ra > r && n > 0) {
      ra--;
      switch (*ra) {
      case ')': n++; break;
      case '(': n--; break;
      }
    }
  }

  while (*pa != '\0')
    pa++;
  while (*pa != ')' && pa > p)
    pa--;
  if (*pa == ')') {
    n = 1;
    while (pa > p && n > 0) {
      pa--;
      switch (*pa) {
      case ')': n++; break;
      case '(': n--; break;
      }
    }
  }
  
  /*
    printf("argument list for %s is '%s'\n", r, ra);
    printf("argument list for %s is '%s'\n", p, pa);
  */

  /* compare return types accepting only an exact match */
  n = (int)((unsigned long)ra - (unsigned long)r) + 1;
  if (n != ((int)((unsigned long)pa - (unsigned long)p) + 1) ||
      strncmp(ra, pa, n))
    return FALSE;

  
  /* compare argument lists */
  r = ra;
  p = pa;
  while (*r && *p) {
    SKIP_WHITESPACE(r);
    SKIP_WHITESPACE(p);
    
    /* scan until a mismatch */
    if (*r && *r == *p) {
      r++, p++;
      continue;
    }      
    
    if (!*r || !*p)
      return FALSE;

    /*printf("before p: '%s', r: '%s'\n", p, r);*/

    /* skip potential/probable variable names */
    /* notes: we cannot handle both having variable names (that mismatch) */
    if (NAME_START(*p) && !NAME_START(*r) && 
	(r[-1] == ' ' || r[-1] == '*' || *r == ')'))
      SKIP_NAME(p);
    else if (NAME_START(*r) && !NAME_START(*p) &&
	     (p[-1] == ' ' || p[-1] == '*' || *p == ')'))
      SKIP_NAME(r);

    /*printf("after p: '%s', r: '%s'\n", p, r);*/

    if (*r != *p)
      return FALSE;
  }
  
  if (!*r && !*p)
    return TRUE;
  else
    return FALSE;
}


static gboolean
ohm_module_resolve_methods(OhmModule *module)
{
  ohm_method_t *method, *m;
  GSList       *l;
  OhmPlugin    *plugin;
  const gchar  *name;
  gchar        *key;
  int           plen, slen, failed;
  
  
  /* construct a symbol table of exported methods */
  symtable = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
  if (symtable == NULL)
    g_error("Failed to allocate method symbol table.");
  
  for (l = module->priv->plugins; l != NULL; l = l->next) {
    plugin = (OhmPlugin *)l->data;
    name   = ohm_plugin_get_name(plugin);
    plen   = strlen(name);

    for (method=ohm_plugin_exports(plugin); method && method->name; method++) {
      if (strchr(method->name, '.') != NULL) {
	g_warning("Invalid exported method name %s from plugin %s.",
		  method->name, name);
	continue;
      }

      slen = plen + 1 + strlen(method->name);
      key  = g_new0(gchar, slen + 1);
      if (key == NULL)
	g_error("Failed to allocate method symbol table key.");
      sprintf(key, "%s.%s", name, method->name);

      if (g_hash_table_lookup(symtable, key) != NULL)
	g_error("Method %s multiply defined.", key);

      method->plugin = plugin;
      g_hash_table_insert(symtable, key, method);
      ohm_debug("%s: method %s exported as %p", name, method->name,method->ptr);
    }
  }
  
  /* resolve required methods */
  failed = FALSE;
  for (l = module->priv->plugins; l != NULL; l = l->next) {
    plugin = (OhmPlugin *)l->data;
    name   = ohm_plugin_get_name(plugin);
    plen   = strlen(name);

    for (method = ohm_plugin_imports(plugin); method && method->ptr; method++) {
      if ((key = (gchar *)method->name) == NULL) {
	g_warning("plugin %s tries to require a NULL method", name);
	continue;
      }

      if ((m = g_hash_table_lookup(symtable, key)) == NULL) {
	g_warning("Could not resolve method %s.", key);
	failed = TRUE;
	continue;
      }
      
      if (!ohm_check_method_signature(method->signature, m->signature)) {
	g_warning("Incompatible signatures for method %s expected by %s.",
		  m->name, name);
	g_warning("%s vs. %s", method->signature, m->signature);
	failed = TRUE;
	continue;
      }
      
      *(void **)method->ptr = m->ptr;
      method->plugin        = OHM_PLUGIN(g_object_ref(m->plugin));
      ohm_debug("%s: method %s resolved to %p...", name, key, m->ptr);
    }
  }

  if (failed)
    g_error("Fatal method resolving errors encountered.");
  
  return TRUE;
}


/**
 * find_matching_method
 **/
static gboolean
find_matching_method(gpointer key, gpointer value, gpointer data)
{
  char         *name   = (char *)key;
  ohm_method_t *method = (ohm_method_t *)value;
  ohm_method_t *req    = (ohm_method_t *)data;
  char         *base   = strchr(name, '.');
  
  if (base != NULL)
    base++;

  if (!strcmp(name, req->name) || (base != NULL && !strcmp(base, req->name))) {
    if (req->signature == NULL ||
	ohm_check_method_signature(method->signature, req->signature)) {
      
      req->ptr  = method->ptr;
      req->name = method->name;
      
      if (!req->signature)
	req->signature = method->signature;
      
      return TRUE;
    }
  }

  return FALSE;
}


/**
 * ohm_module_find_method:
 **/
gboolean
ohm_module_find_method(char *name, char **sigptr, void **funcptr)
{
  ohm_method_t *m, method;

  if ((m = g_hash_table_lookup(symtable, name)) == NULL) {
    method.name      = name;
    method.signature = (sigptr && *sigptr ? *sigptr : NULL);
    method.plugin    = NULL;
    
    if (g_hash_table_find(symtable, find_matching_method, (gpointer)&method))
      m = &method;
  }
   
  if (m != NULL) {
    if (funcptr != NULL)
      *funcptr = m->ptr;
    if (sigptr != NULL)
      *sigptr = (char *)m->signature;
    
    if (m->plugin != NULL) {
      ohm_debug("refcounting plugin %s because of dynamic method lookup",
		ohm_plugin_get_name(m->plugin));
      g_object_ref(m->plugin);
    }


    return TRUE;
  }
  
  return FALSE;
}



static gboolean
ohm_module_dbus_setup(OhmModule *module)
{
  ohm_dbus_method_t *m;
  ohm_dbus_signal_t *s;

  GSList       *l;
  OhmPlugin    *plugin;
  const gchar  *name;

  for (l = module->priv->plugins; l != NULL; l = l->next) {
    plugin = (OhmPlugin *)l->data;
    name   = ohm_plugin_get_name(plugin);
    
    for (m = plugin->dbus_methods; m && m->name; m++)
      if (!ohm_dbus_add_method(m))
	g_error("Failed to register DBUS method %s for plugin %s.",
		m->name, name);

    for (s = plugin->dbus_signals; s && s->signal; s++)
      if (!ohm_dbus_add_signal(s->sender, s->interface, s->signal,
			       s->path,
			       s->handler, s->data))
	g_error("Failed to register DBUS signal handler %s for plugin %s.",
		s->signal, name);
  }
  
  return TRUE;
}


static gboolean
ohm_module_dbus_cleanup(OhmModule *module)
{
  ohm_dbus_method_t *m;
  ohm_dbus_signal_t *s;

  GSList       *l;
  OhmPlugin    *plugin;
  const gchar  *name;

  for (l = module->priv->plugins; l != NULL; l = l->next) {
    plugin = (OhmPlugin *)l->data;
    name   = ohm_plugin_get_name(plugin);
    
    for (m = plugin->dbus_methods; m && m->name; m++)
      if (!ohm_dbus_del_method(m))
	g_warning("Failed to unregister DBUS method %s:%s.%s.",
		  name, m->path, m->name);
    for (s = plugin->dbus_signals; s && s->signal; s++)
      ohm_dbus_del_signal(s->sender, s->interface, s->signal, s->path,
			  s->handler, s->data);
  }
  
  return TRUE;
}


static void
ohm_module_finalize (GObject *object)
{
	OhmModule *module;
	GSList *l;
	OhmPlugin *plugin;

	g_return_if_fail (object != NULL);
	g_return_if_fail (OHM_IS_MODULE (object));
	module = OHM_MODULE (object);

	/* clean up DBUS methods and signals */
	ohm_module_dbus_cleanup(module);

	g_hash_table_foreach_remove (module->priv->interested, (GHRFunc) free_notify_list, NULL);
	g_hash_table_destroy (module->priv->interested);
	g_object_unref (module->priv->conf);

	/* unref each plugin */
	for (l=module->priv->plugins; l != NULL; l=l->next) {
		plugin = (OhmPlugin *) l->data;
		g_object_unref (plugin);
	}
	g_slist_free (module->priv->plugins);

	g_hash_table_destroy(symtable);
	symtable = NULL;

	g_return_if_fail (module->priv != NULL);
	G_OBJECT_CLASS (ohm_module_parent_class)->finalize (object);
}


/**
 * ohm_module_class_init:
 **/
static void
ohm_module_class_init (OhmModuleClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize	   = ohm_module_finalize;

	g_type_class_add_private (klass, sizeof (OhmModulePrivate));
}

/**
 * ohm_module_init:
 **/
static void
ohm_module_init (OhmModule *module)
{
	guint i;
	GSList *l;
	OhmPlugin *plugin;
	const gchar *name;
	GError *error;
	gboolean ret;

	module->priv = OHM_MODULE_GET_PRIVATE (module);

	module->priv->interested = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, NULL);

	module->priv->conf = ohm_conf_new ();
	g_signal_connect (module->priv->conf, "key-changed",
			  G_CALLBACK (key_changed_cb), module);

	/* read the defaults in from modules.ini */
	ohm_module_read_defaults (module);

	/* Keep trying to empty both require and suggested lists.
	 * We could have done this recursively, but that is really bad for the stack.
	 * We also have to keep in mind the lists may be being updated by plugins as we load them */
	i = 1;
	while (module->priv->mod_require != NULL ||
	       module->priv->mod_suggest != NULL) {
		ohm_debug ("module add iteration #%i", i++);
		ohm_module_add_all_plugins (module);
		if (i > 10) {
			g_error ("Module add too complex, please file a bug");
		}
	}
	g_slist_free (module->priv->mod_prevent);
	g_slist_free (module->priv->mod_loaded);
	g_strfreev (module->priv->modules_required);
	g_strfreev (module->priv->modules_suggested);
	g_strfreev (module->priv->modules_banned);

	/* resolve method cross-references */
	ohm_module_resolve_methods(module);

	/* set up DBUS methods and signals */
	ohm_module_dbus_setup(module);

	/* add defaults for each plugin before the initialization*/
	ohm_debug ("loading plugin defaults");
	for (l=module->priv->plugins; l != NULL; l=l->next) {
		plugin = (OhmPlugin *) l->data;
		name = ohm_plugin_get_name (plugin);
		ohm_debug ("load defaults %s", name);

		/* load defaults from disk */
		error = NULL;
		ret = ohm_conf_load_defaults (module->priv->conf, name, &error);
		if (ret == FALSE) {
			ohm_debug ("not defaults for %s: %s", name, error->message);
			g_error_free (error);
		}

		/* load plugin parameters */
		error = NULL;
		ret = ohm_plugin_load_params (plugin, &error);
		if (ret == FALSE) {
		        g_error ("failed to load plugin parameters for %s: %s",
				 name, error->message);
		}
	}


	/* determine plugin load order satisfying plugin dependencies */
	if (!ohm_module_reorder_plugins(module))
	  g_error("Cannot load plugins.");

	ohm_conf_set_initializing (module->priv->conf, TRUE);
	/* initialize each plugin */
	ohm_debug ("starting plugin initialization");
	for (l=module->priv->plugins; l != NULL; l=l->next) {
		plugin = (OhmPlugin *) l->data;
		name = ohm_plugin_get_name (plugin);
		ohm_debug ("initialize %s", name);
		ohm_plugin_initialize (plugin);
	}
	ohm_conf_set_initializing (module->priv->conf, FALSE);
}

/**
 * ohm_module_new:
 **/
OhmModule *
ohm_module_new (void)
{
	OhmModule *module;
	module = g_object_new (OHM_TYPE_MODULE, NULL);
	return OHM_MODULE (module);
}
