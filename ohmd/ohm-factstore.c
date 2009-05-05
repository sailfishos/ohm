/*
 * This file is part of Ohm
 *
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Marc-Andre Lureau <marc-andre.lureau@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include <ohm/ohm-factstore.h>

enum  {
	OHM_STRUCTURE_DUMMY_PROPERTY,
	OHM_STRUCTURE_QNAME,
	OHM_STRUCTURE_NAME
};
static void _ohm_structure_unset_and_free (void* p);
static void ohm_structure_real_qset (OhmStructure* self, GQuark field, GValue* value);
static void _ohm_structure_value_to_string_gvalue_transform (const GValue* src_value, GValue* dest_value);
static GObject * ohm_structure_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer ohm_structure_parent_class = NULL;
static void ohm_structure_dispose (GObject * obj);
struct _OhmPatternPrivate {
	OhmFactStoreView* _view;
	OhmFact* _fact;
};

#define OHM_PATTERN_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), OHM_TYPE_PATTERN, OhmPatternPrivate))
enum  {
	OHM_PATTERN_DUMMY_PROPERTY,
	OHM_PATTERN_VIEW,
	OHM_PATTERN_FACT
};
struct _OhmPatternMatchPrivate {
	OhmFact* _fact;
	OhmPattern* _pattern;
	OhmFactStoreEvent _event;
};

#define OHM_PATTERN_MATCH_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), OHM_PATTERN_TYPE_MATCH, OhmPatternMatchPrivate))
enum  {
	OHM_PATTERN_MATCH_DUMMY_PROPERTY,
	OHM_PATTERN_MATCH_FACT,
	OHM_PATTERN_MATCH_PATTERN,
	OHM_PATTERN_MATCH_EVENT
};
static void ohm_pattern_match_set_fact (OhmPatternMatch* self, OhmFact* value);
static void ohm_pattern_match_set_pattern (OhmPatternMatch* self, OhmPattern* value);
static void ohm_pattern_match_set_event (OhmPatternMatch* self, OhmFactStoreEvent value);
static gpointer ohm_pattern_match_parent_class = NULL;
static void ohm_pattern_match_dispose (GObject * obj);
static gpointer ohm_pattern_parent_class = NULL;
static void ohm_pattern_dispose (GObject * obj);
struct _OhmFactPrivate {
	OhmFactStore* _fact_store;
};

#define OHM_FACT_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), OHM_TYPE_FACT, OhmFactPrivate))
enum  {
	OHM_FACT_DUMMY_PROPERTY,
	OHM_FACT_FACT_STORE
};
static void ohm_fact_real_qset (OhmStructure* base, GQuark field, GValue* value);
static gpointer ohm_fact_parent_class = NULL;
static void ohm_fact_dispose (GObject * obj);
struct _OhmFactStorePrivate {
	GSList* known_facts_qname;
	GData* interest;
};

#define OHM_FACT_STORE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), OHM_TYPE_FACT_STORE, OhmFactStorePrivate))
enum  {
	OHM_FACT_STORE_DUMMY_PROPERTY
};
static void _ohm_fact_store_update_views (OhmFactStore* self, OhmFact* fact, OhmFactStoreEvent event, GQuark field, GValue *value);
static gboolean ohm_fact_store_insert_internal (OhmFactStore* self, OhmFact* fact);
static gboolean ohm_fact_store_remove_internal (OhmFactStore* self, OhmFact* fact);
static void _g_slist_free_g_object_unref (GSList* self);
static void _ohm_fact_store_delete_func (GSList* l);
static void ohm_fact_store_set_view_interest (OhmFactStore* self, OhmFactStoreView* v);
struct _OhmFactStoreChangeSetPrivate {
	GSList* _matches;
};

#define OHM_FACT_STORE_CHANGE_SET_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), OHM_FACT_STORE_TYPE_CHANGE_SET, OhmFactStoreChangeSetPrivate))
enum  {
	OHM_FACT_STORE_CHANGE_SET_DUMMY_PROPERTY,
	OHM_FACT_STORE_CHANGE_SET_MATCHES
};
static gpointer ohm_fact_store_change_set_parent_class = NULL;
static void ohm_fact_store_change_set_dispose (GObject * obj);
struct _OhmFactStoreSimpleViewPrivate {
	GObject* _listener;
	OhmFactStore* _fact_store;
};

#define OHM_FACT_STORE_SIMPLE_VIEW_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), OHM_FACT_STORE_TYPE_SIMPLE_VIEW, OhmFactStoreSimpleViewPrivate))
enum  {
	OHM_FACT_STORE_SIMPLE_VIEW_DUMMY_PROPERTY,
	OHM_FACT_STORE_SIMPLE_VIEW_LISTENER,
	OHM_FACT_STORE_SIMPLE_VIEW_FACT_STORE
};
static GObject * ohm_fact_store_simple_view_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer ohm_fact_store_simple_view_parent_class = NULL;
static void ohm_fact_store_simple_view_dispose (GObject * obj);
enum  {
	OHM_FACT_STORE_TRANSACTION_DUMMY_PROPERTY
};
static void _g_slist_free_ohm_pair_free (GSList* self);
static void _g_slist_free_ohm_fact_store_transaction_cow_free (GSList* self);
static OhmFactStoreTransaction* ohm_fact_store_transaction_new (OhmFactStore* fact_store, GObject* listener);
static gboolean _ohm_fact_store_transaction_active(OhmFactStore *self);
static gboolean _ohm_fact_store_transaction_rolledback(OhmFactStore *self);
static gpointer ohm_fact_store_transaction_parent_class = NULL;
static void ohm_fact_store_transaction_dispose (GObject * obj);
enum  {
	OHM_FACT_STORE_VIEW_DUMMY_PROPERTY
};
static OhmFactStoreView* ohm_fact_store_view_new (OhmFactStore* fact_store, GObject* listener);
static gpointer ohm_fact_store_view_parent_class = NULL;
static void ohm_fact_store_view_dispose (GObject * obj);
static gpointer ohm_fact_store_parent_class = NULL;
static void ohm_fact_store_dispose (GObject * obj);
struct _OhmRulePrivate {
	OhmFactStoreView* _view;
};

#define OHM_RULE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), OHM_TYPE_RULE, OhmRulePrivate))
enum  {
	OHM_RULE_DUMMY_PROPERTY,
	OHM_RULE_VIEW,
	OHM_RULE_FACT_STORE
};
OhmFactStore* ohm_fs = NULL;


static void g_cclosure_user_marshal_VOID__OBJECT_UINT_POINTER (GClosure * closure, GValue * return_value, guint n_param_values, const GValue * param_values, gpointer invocation_hint, gpointer marshal_data);

/**
 * ohm_pair_new:
 * @first: pointer to first, weak ref
 * @second: pointer to second, weak ref
 *
 * <note>
 * <para>
 * </para>
 * </note>
 *
 * Returns: a new Pair&lt;first,second&gt;
 **/
OhmPair* ohm_pair_new (gpointer first, gpointer second,
		       GDestroyNotify first_destroy_func, GDestroyNotify second_destroy_func) {
	OhmPair* self;

	self = g_slice_new0 (OhmPair);

	self->first = first;
	self->second = second;
	self->first_destroy_func = first_destroy_func;
	self->second_destroy_func = second_destroy_func;

	return self;
}


/**
 * ohm_pair_free:
 * @self:  a pair
 *
 * The function will not free @first and @second. FIXME
 *
 *
 *
 * OhmStructure:
 * @fields:  the list of field names, as #GQuark, in #SList->@data
 *
 * #OhmStructure is used as a dynamic structure (like
 * GstStructure). it can have multiple dynamic fields of
 * different types.
 **/
void ohm_pair_free (OhmPair* self) {
	if (self->first_destroy_func)
	  self->first_destroy_func (self->first);

	if (self->second_destroy_func)
	  self->second_destroy_func (self->second);

	g_slice_free (OhmPair, self);
}


/**
 * ohm_structure_new:
 * @name: the name of the structure
 *
 * Create a new #OhmStructure
 *
 * Returns: a new, 0-field, #OhmStructure with the name @name.
 **/
OhmStructure* ohm_structure_new (const char* name) {
	OhmStructure * self;

	g_return_val_if_fail (name != NULL, NULL);

	self = g_object_new (OHM_TYPE_STRUCTURE,
			     "name", name,
			     NULL);

	return self;
}


static void _ohm_structure_unset_and_free (void* p) {
	GValue* v;

	v = (GValue*) p;

	g_value_unset (v);
	g_free (v);
}

/**
 * ohm_structure_qset:
 * @self: a #OhmStructure
 * @field: the #GQuark name of the field to set
 * @value: a #GValue with an arbitrary type. If %NULL, then the @field is removed.
 *
 * Set a @field to @value.
 * @value should be allocated by the caller. It will be freed when #OhmStructure is destroyed or the field is removed.
 **/
static void ohm_structure_real_qset (OhmStructure* self, GQuark field, GValue* value) {
	g_return_if_fail (OHM_IS_STRUCTURE (self));

	if (value == NULL) {
		self->fields = g_slist_remove (self->fields, GINT_TO_POINTER (field));
		g_object_set_qdata (G_OBJECT (self), field, NULL);
	} else {
		if (g_slist_find (self->fields, GINT_TO_POINTER (field)) == NULL) {
			self->fields = g_slist_append (self->fields, GINT_TO_POINTER (field));
		}/*  else { */
/* 			g_debug ("Field already set, reset"); */
/* 		} */
		g_object_set_qdata_full (G_OBJECT (self), field, value, _ohm_structure_unset_and_free);
	}
}


void ohm_structure_qset (OhmStructure* self, GQuark field, GValue* value) {
	OHM_STRUCTURE_GET_CLASS (self)->qset (self, field, value);
}


/**
 * ohm_structure_qget:
 * @self: a #OhmStructure
 * @field: the #GQuark name of the field to get
 *
 * Returns: The field value or %NULL if the field does not exist.
 **/
GValue* ohm_structure_qget (OhmStructure* self, GQuark field) {
	return ((GValue*) g_object_get_qdata (G_OBJECT (self), field));
}


/**
 * ohm_structure_set:
 * @self: a #OhmStructure
 * @field: the name of the field to set
 * @value: a #GValue with an arbitrary type. If %NULL, then the @field is removed.
 *
 * Set a @field to @value.
 * @value should be allocated by the caller. It will be freed when #OhmStructure is destroyed or the field is removed.
 **/
void ohm_structure_set (OhmStructure* self, const char* field_name, GValue* value) {
	g_return_if_fail (OHM_IS_STRUCTURE (self));
	g_return_if_fail (field_name != NULL);

	ohm_structure_qset (self, g_quark_from_string (field_name), value);
}


/**
 * ohm_structure_get:
 * @self: a #OhmStructure
 * @field: the name of the field to get
 *
 * Returns: The field value or %NULL if the field does not exist.
 **/
GValue* ohm_structure_get (OhmStructure* self, const char* field_name) {
	return ((GValue*) g_object_get_data (G_OBJECT (self), field_name));
}


/**
 * ohm_structure_to_string:
 * @self: a #OhmStructure to dump to string
 *
 * Returns: an allocated string of debug/introspection information. (From vala version)
 **/
char* ohm_structure_to_string (OhmStructure* self) {
	char* ret;
	gboolean first;
	char* _tmp4;
	GSList* q_collection;
	GSList* q_it;

	g_return_val_if_fail (OHM_IS_STRUCTURE (self), NULL);
	ret = g_strdup_printf ("%s (", ohm_structure_get_name (self));
	first = TRUE;

	q_collection = self->fields;
	for (q_it = q_collection; q_it != NULL; q_it = q_it->next) {
	  GQuark q;
	  q = GPOINTER_TO_INT (q_it->data);
	  {
	    GValue* v;
	    char* _tmp3;
	    char* _tmp2;
	    char* _tmp1;
	    if (!first) {
	      char* _tmp0;
	      _tmp0 = NULL;
	      ret = (_tmp0 = g_strconcat (ret, (", "), NULL), (ret = (g_free (ret), NULL)), _tmp0);
	    } else {
	      first = FALSE;
	    }
	    v = g_object_get_qdata (G_OBJECT (self), q);
	    _tmp3 = NULL;
	    _tmp2 = NULL;
	    _tmp1 = NULL;
	    ret = (_tmp3 = g_strconcat (ret, (_tmp2 = (g_strdup_printf ("%s = %s", g_quark_to_string (q), (_tmp1 = g_strdup_value_contents (v))))), NULL), (ret = (g_free (ret), NULL)), _tmp3);
	    _tmp2 = (g_free (_tmp2), NULL);
	    _tmp1 = (g_free (_tmp1), NULL);
	  }
	}

	_tmp4 = NULL;
	return (_tmp4 = g_strconcat (ret, ")", NULL), (ret = (g_free (ret), NULL)), _tmp4);
}


/**
 * ohm_structure_value_to_string:
 * @src: a @src value
 * @dest: a @dest value
 *
 * Convert a #GValue of type #OhmStructure to a string #GValue.
 **/
void ohm_structure_value_to_string (const GValue* src, GValue* dest) {
	OhmStructure* s;
	char* structstr;

	s = ohm_value_get_structure (src);
	g_return_if_fail (s != NULL);

	structstr = ohm_structure_to_string (s);
	g_value_set_string (dest, structstr);
	g_free (structstr);
	g_object_unref (s);
}


GQuark ohm_structure_get_qname (OhmStructure* self) {
	g_return_val_if_fail (OHM_IS_STRUCTURE (self), 0U);

	return self->_name;
}


const char* ohm_structure_get_name (OhmStructure* self) {
	g_return_val_if_fail (OHM_IS_STRUCTURE (self), NULL);

	return g_quark_to_string (self->_name);
}


void ohm_structure_set_name (OhmStructure* self, const char* value) {
	g_return_if_fail (OHM_IS_STRUCTURE (self));

	self->_name = g_quark_from_string (value);
}


static void _ohm_structure_value_to_string_gvalue_transform (const GValue* src_value, GValue* dest_value) {
	ohm_structure_value_to_string (src_value, dest_value);
}


static GObject * ohm_structure_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	OhmStructureClass * klass;
	GObjectClass * parent_class;
	OhmStructure * self;

	klass = OHM_STRUCTURE_CLASS (g_type_class_peek (OHM_TYPE_STRUCTURE));
	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);

	self = OHM_STRUCTURE (obj);

	g_value_register_transform_func (OHM_TYPE_STRUCTURE, G_TYPE_STRING, _ohm_structure_value_to_string_gvalue_transform);

	return obj;
}


static void ohm_structure_get_property (GObject * object, guint property_id, GValue * value, GParamSpec * pspec) {
	OhmStructure * self;

	self = OHM_STRUCTURE (object);

	switch (property_id) {
	case OHM_STRUCTURE_QNAME:
	  g_value_set_uint (value, ohm_structure_get_qname (self));
	  break;
	case OHM_STRUCTURE_NAME:
	  g_value_set_string (value, ohm_structure_get_name (self));
	  break;
	default:
	  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	  break;
	}
}


static void ohm_structure_set_property (GObject * object, guint property_id, const GValue * value, GParamSpec * pspec) {
	OhmStructure * self;

	self = OHM_STRUCTURE (object);

	switch (property_id) {
	case OHM_STRUCTURE_NAME:
	  ohm_structure_set_name (self, g_value_get_string (value));
	  break;
	default:
	  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	  break;
	}
}


static void ohm_structure_class_init (OhmStructureClass * klass) {
	ohm_structure_parent_class = g_type_class_peek_parent (klass);

	G_OBJECT_CLASS (klass)->get_property = ohm_structure_get_property;
	G_OBJECT_CLASS (klass)->set_property = ohm_structure_set_property;
	G_OBJECT_CLASS (klass)->constructor = ohm_structure_constructor;
	G_OBJECT_CLASS (klass)->dispose = ohm_structure_dispose;
	OHM_STRUCTURE_CLASS (klass)->qset = ohm_structure_real_qset;

	/**
	 * OhmStructure:qname:
	 *
	 * The #GQuark name of the #OhmStructure itself, like a dynamic type name.
	 **/
	g_object_class_install_property (G_OBJECT_CLASS (klass), OHM_STRUCTURE_QNAME,
					 g_param_spec_uint ("qname", "qname", "qname", 0, G_MAXUINT, 0U,
							    G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE));

	/**
	 * OhmStructure:name:
	 *
	 * The name of the #OhmStructure itself. See #OhmStructure:qname.
	 **/
	g_object_class_install_property (G_OBJECT_CLASS (klass), OHM_STRUCTURE_NAME,
					 g_param_spec_string ("name", "name", "name", NULL,
							      G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));
}


static void ohm_structure_init (OhmStructure * self) {
	self->fields = NULL;
}


static void ohm_structure_dispose (GObject * obj) {
	OhmStructure * self;

	self = OHM_STRUCTURE (obj);

	if (self->fields != NULL) {
	  g_slist_free (self->fields);
	  self->fields = NULL;
	}

	G_OBJECT_CLASS (ohm_structure_parent_class)->dispose (obj);
}


GType ohm_structure_get_type (void) {
	static GType ohm_structure_type_id = 0;

	if (G_UNLIKELY (ohm_structure_type_id == 0)) {
		static const GTypeInfo g_define_type_info = {
		  sizeof (OhmStructureClass),
		  (GBaseInitFunc) NULL,
		  (GBaseFinalizeFunc) NULL,
		  (GClassInitFunc) ohm_structure_class_init,
		  (GClassFinalizeFunc) NULL,
		  NULL,
		  sizeof (OhmStructure),
		  0,
		  (GInstanceInitFunc) ohm_structure_init
		};

		ohm_structure_type_id = g_type_register_static (G_TYPE_OBJECT, "OhmStructure", &g_define_type_info, 0);
	}

	return ohm_structure_type_id;
}


/**
 * ohm_pattern_new:
 * @name: the fact name to match with the pattern
 *
 * Create a new #OhmPattern.
 *
 * Returns: a fresh new, 0-field, #OhmPattern with for the name @name.
 **/
OhmPattern* ohm_pattern_new (const char* name) {
	return g_object_new (OHM_TYPE_PATTERN, "name", name, NULL);
}


/**
 * ohm_pattern_new_for_fact:
 * @fact: the fact to match with this pattern.
 *
 * Create a new #OhmPattern that will match @fact.
 *
 * Returns: a new #OhmPattern that will match only @fact.
 **/
OhmPattern* ohm_pattern_new_for_fact (OhmFact* fact) {
	OhmPattern* self;

	self = g_object_new (OHM_TYPE_PATTERN,
			     "name",
			     ohm_structure_get_name (OHM_STRUCTURE (fact)),
			     NULL);
	ohm_pattern_set_fact (self, fact);

	return self;
}


/**
 * ohm_pattern_match:
 * @self: the pattern
 * @fact: the fact to match (not %NULL)
 * @event: the event that caused the match evaluation (ex: %OHM_FACT_STORE_EVENT_LOOKUP)
 *
 * This method is used to get the facts from the #OhmFactStore, mainly
 * for debugging or by the #OhmFactStoreView views. You usually don't
 * have to match facts yourself, but instead rely on #OhmFactStoreView
 * functionnality.
 *
 * Returns: a new #OhmPatternMatch if the @fact matches the pattern @self, or %NULL.
 **/
OhmPatternMatch* ohm_pattern_match (OhmPattern* self, OhmFact* fact, OhmFactStoreEvent event) {
	GSList* q_collection;
	GSList* q_it;

	g_return_val_if_fail (OHM_IS_PATTERN (self), NULL);
	g_return_val_if_fail (OHM_IS_FACT (fact), NULL);

	if (self->priv->_fact == fact) {
		return ohm_pattern_match_new (fact, self, event);
	}

	if (ohm_structure_get_qname (OHM_STRUCTURE (fact)) != ohm_structure_get_qname (OHM_STRUCTURE (self))) {
		return NULL;
	}

	q_collection = OHM_STRUCTURE (self)->fields;

	for (q_it = q_collection; q_it != NULL; q_it = q_it->next) {
	  GQuark q;
	  GValue* vthis;
	  GValue* vfact;

	  q = GPOINTER_TO_INT (q_it->data);
	  
	  vthis = g_object_get_qdata (G_OBJECT (self), q);
	  vfact = g_object_get_qdata (G_OBJECT (fact), q);

	  if ((vthis != NULL && vfact == NULL) || (vthis == NULL && vfact != NULL)) {
	    return NULL;
	  }
	  
	  if (vthis != NULL && vfact != NULL) {
	    if (G_VALUE_TYPE (vthis) != G_VALUE_TYPE (vfact)) {
	      return NULL;
	    } else {
	      if (ohm_value_cmp (vthis, vfact) != 0) {
		return NULL;
	      }
	    }
	  }
	}
	
	return ohm_pattern_match_new (fact, self, event);
}


OhmFactStoreView* ohm_pattern_get_view (OhmPattern* self) {
	g_return_val_if_fail (OHM_IS_PATTERN (self), NULL);

	return self->priv->_view;
}


void ohm_pattern_set_view (OhmPattern* self, OhmFactStoreView* value) {
	g_return_if_fail (OHM_IS_PATTERN (self));

	self->priv->_view = value;

	if (self->priv->_view != NULL) {
	  g_object_add_weak_pointer (G_OBJECT (self->priv->_view), (void*)&self->priv->_view);
	}
}


OhmFact* ohm_pattern_get_fact (OhmPattern* self) {
	g_return_val_if_fail (OHM_IS_PATTERN (self), NULL);

	return self->priv->_fact;
}


void ohm_pattern_set_fact (OhmPattern* self, OhmFact* value) {
	g_return_if_fail (OHM_IS_PATTERN (self));

	if (self->priv->_fact != NULL) {
	  g_object_unref (self->priv->_fact);
	}

	self->priv->_fact = g_object_ref (value);
}


/**
 * ohm_pattern_match_new:
 * @fact: a fact (not %NULL)
 * @pattern: a pattern.
 * @event: the match event.
 *
 * Create a new #OhmPatternMatch to represent a match (used internally be ohm_pattern_match () ).
 *
 * Returns: a new #OhmPatternMatch.
 **/
OhmPatternMatch* ohm_pattern_match_new (OhmFact* fact, OhmPattern* pattern, OhmFactStoreEvent event) {
	return g_object_new (OHM_PATTERN_TYPE_MATCH,
			     "fact", fact,
			     "pattern", pattern,
			     "event", event,
			     NULL);
}


/**
 * ohm_pattern_match_to_string:
 * @self: a #OhmPatternMatch to dump to string
 *
 * Returns: an allocated string of debug/introspection information.
 **/
char* ohm_pattern_match_to_string (OhmPatternMatch* self) {
	char* fact;
	char* pattern;
	char* ret;

	g_return_val_if_fail (OHM_PATTERN_IS_MATCH (self), NULL);

	fact = ohm_structure_to_string (OHM_STRUCTURE (self->priv->_fact));
	pattern = ohm_structure_to_string (OHM_STRUCTURE (self->priv->_pattern));

	ret = g_strdup_printf ("a match: %s, %s", fact, pattern);

	if (fact) {
	  g_free (fact);
	}

	if (pattern) {
	  g_free (pattern);
	}

	return ret;
}


OhmFact* ohm_pattern_match_get_fact (OhmPatternMatch* self) {
	g_return_val_if_fail (OHM_PATTERN_IS_MATCH (self), NULL);

	return self->priv->_fact;
}


static void ohm_pattern_match_set_fact (OhmPatternMatch* self, OhmFact* value) {
	g_return_if_fail (OHM_PATTERN_IS_MATCH (self));

	if (self->priv->_fact != NULL) {
	  g_object_unref (self->priv->_fact);
	}

	self->priv->_fact = g_object_ref (value);
}


OhmPattern* ohm_pattern_match_get_pattern (OhmPatternMatch* self) {
	g_return_val_if_fail (OHM_PATTERN_IS_MATCH (self), NULL);

	return self->priv->_pattern;
}


static void ohm_pattern_match_set_pattern (OhmPatternMatch* self, OhmPattern* value) {
	g_return_if_fail (OHM_PATTERN_IS_MATCH (self));

	if (self->priv->_pattern != NULL) {
	  g_object_unref (self->priv->_pattern);
	}

	self->priv->_pattern = g_object_ref (value);
}


OhmFactStoreEvent ohm_pattern_match_get_event (OhmPatternMatch* self) {
	g_return_val_if_fail (OHM_PATTERN_IS_MATCH (self), 0);

	return self->priv->_event;
}


static void ohm_pattern_match_set_event (OhmPatternMatch* self, OhmFactStoreEvent value) {
	g_return_if_fail (OHM_PATTERN_IS_MATCH (self));

	self->priv->_event = value;
}


static void ohm_pattern_match_get_property (GObject * object, guint property_id, GValue * value, GParamSpec * pspec) {
	OhmPatternMatch * self;

	self = OHM_PATTERN_MATCH (object);

	switch (property_id) {
	case OHM_PATTERN_MATCH_FACT:
	  g_value_set_object (value, ohm_pattern_match_get_fact (self));
	  break;
	case OHM_PATTERN_MATCH_PATTERN:
	  g_value_set_object (value, ohm_pattern_match_get_pattern (self));
	  break;
	case OHM_PATTERN_MATCH_EVENT:
	  g_value_set_enum (value, ohm_pattern_match_get_event (self));
	  break;
	default:
	  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	  break;
	}
}


static void ohm_pattern_match_set_property (GObject * object, guint property_id, const GValue * value, GParamSpec * pspec) {
	OhmPatternMatch * self;

	self = OHM_PATTERN_MATCH (object);

	switch (property_id) {
	case OHM_PATTERN_MATCH_FACT:
	  ohm_pattern_match_set_fact (self, g_value_get_object (value));
	  break;
	case OHM_PATTERN_MATCH_PATTERN:
	  ohm_pattern_match_set_pattern (self, g_value_get_object (value));
	  break;
	case OHM_PATTERN_MATCH_EVENT:
	  ohm_pattern_match_set_event (self, g_value_get_enum (value));
	  break;
	default:
	  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	  break;
	}
}


static void ohm_pattern_match_class_init (OhmPatternMatchClass * klass) {
	ohm_pattern_match_parent_class = g_type_class_peek_parent (klass);

	g_type_class_add_private (klass, sizeof (OhmPatternMatchPrivate));
	G_OBJECT_CLASS (klass)->get_property = ohm_pattern_match_get_property;
	G_OBJECT_CLASS (klass)->set_property = ohm_pattern_match_set_property;
	G_OBJECT_CLASS (klass)->dispose = ohm_pattern_match_dispose;

	/**
	 * OhmPatternMatch:fact:
	 *
	 * The :fact matched.
	 **/
	g_object_class_install_property (G_OBJECT_CLASS (klass),
					 OHM_PATTERN_MATCH_FACT,
					 g_param_spec_object ("fact", "fact", "fact", OHM_TYPE_FACT,
							      G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

	/**
	 * OhmPatternMatch:pattern:
	 *
	 * The :pattern that matched the fact.
	 **/
	g_object_class_install_property (G_OBJECT_CLASS (klass),
					 OHM_PATTERN_MATCH_PATTERN,
					 g_param_spec_object ("pattern", "pattern", "pattern", OHM_TYPE_PATTERN,
							      G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

	/**
	 * OhmPatternMatch:event:
	 *
	 * The :event that caused the match.
	 **/
	g_object_class_install_property (G_OBJECT_CLASS (klass),
					 OHM_PATTERN_MATCH_EVENT,
					 g_param_spec_enum ("event", "event", "event", OHM_FACT_STORE_TYPE_EVENT, 
							    0, G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
}


static void ohm_pattern_match_init (OhmPatternMatch * self) {
	self->priv = OHM_PATTERN_MATCH_GET_PRIVATE (self);
}


static void ohm_pattern_match_dispose (GObject * obj) {
	OhmPatternMatch * self;

	self = OHM_PATTERN_MATCH (obj);

	if (self->priv->_fact != NULL) {
	  g_object_unref (self->priv->_fact);
	  self->priv->_fact = NULL;
	}
	if (self->priv->_pattern != NULL) {
	  g_object_unref (self->priv->_pattern);
	  self->priv->_pattern = NULL;
	}

	G_OBJECT_CLASS (ohm_pattern_match_parent_class)->dispose (obj);
}


GType ohm_pattern_match_get_type (void) {
	static GType ohm_pattern_match_type_id = 0;

	if (G_UNLIKELY (ohm_pattern_match_type_id == 0)) {
	  static const GTypeInfo g_define_type_info = {
	    sizeof (OhmPatternMatchClass),
	    (GBaseInitFunc) NULL,
	    (GBaseFinalizeFunc) NULL,
	    (GClassInitFunc) ohm_pattern_match_class_init,
	    (GClassFinalizeFunc) NULL,
	    NULL,
	    sizeof (OhmPatternMatch),
	    0,
	    (GInstanceInitFunc) ohm_pattern_match_init
	  };

	  ohm_pattern_match_type_id = g_type_register_static (G_TYPE_OBJECT, "OhmPatternMatch", &g_define_type_info, 0);
	}

	return ohm_pattern_match_type_id;
}


static void ohm_pattern_get_property (GObject * object, guint property_id, GValue * value, GParamSpec * pspec) {
	OhmPattern * self;

	self = OHM_PATTERN (object);
	switch (property_id) {
	case OHM_PATTERN_VIEW:
	  g_value_set_object (value, ohm_pattern_get_view (self));
	  break;
	case OHM_PATTERN_FACT:
	  g_value_set_object (value, ohm_pattern_get_fact (self));
	  break;
	default:
	  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	  break;
	}
}


static void ohm_pattern_set_property (GObject * object, guint property_id, const GValue * value, GParamSpec * pspec) {
	OhmPattern * self;

	self = OHM_PATTERN (object);
	switch (property_id) {
	case OHM_PATTERN_VIEW:
	  ohm_pattern_set_view (self, g_value_get_object (value));
	  break;
	case OHM_PATTERN_FACT:
	  ohm_pattern_set_fact (self, g_value_get_object (value));
	  break;
	default:
	  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	  break;
	}
}


static void ohm_pattern_class_init (OhmPatternClass * klass) {
	ohm_pattern_parent_class = g_type_class_peek_parent (klass);

	g_type_class_add_private (klass, sizeof (OhmPatternPrivate));

	G_OBJECT_CLASS (klass)->get_property = ohm_pattern_get_property;
	G_OBJECT_CLASS (klass)->set_property = ohm_pattern_set_property;
	G_OBJECT_CLASS (klass)->dispose = ohm_pattern_dispose;

	/**
	 * OhmPattern:view:
	 *
	 * If the pattern belongs to a view, it returns a weak reference to it, else %NULL.
	 **/
	g_object_class_install_property (G_OBJECT_CLASS (klass), OHM_PATTERN_VIEW,
					 g_param_spec_object ("view", "view", "view", OHM_FACT_STORE_TYPE_VIEW,
							      G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE));

	/**
	 * OhmPattern:fact:
	 *
	 * If the pattern is associated to a specific fact, it returns a reference to it, else %NULL.
	 **/
	g_object_class_install_property (G_OBJECT_CLASS (klass), OHM_PATTERN_FACT,
					 g_param_spec_object ("fact", "fact", "fact", OHM_TYPE_FACT,
							      G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE));
}


static void ohm_pattern_init (OhmPattern * self) {
	self->priv = OHM_PATTERN_GET_PRIVATE (self);
}


static void ohm_pattern_dispose (GObject * obj) {
	OhmPattern * self;

	self = OHM_PATTERN (obj);

	if (ohm_pattern_get_view (self) != NULL) {
	  g_object_remove_weak_pointer (G_OBJECT (self->priv->_view),
					(gpointer)&self->priv->_view);
	}

	if (self->priv->_fact != NULL) {
	  g_object_unref (self->priv->_fact);
	  self->priv->_fact = NULL;
	}

	G_OBJECT_CLASS (ohm_pattern_parent_class)->dispose (obj);
}


GType ohm_pattern_get_type (void) {
	static GType ohm_pattern_type_id = 0;

	if (G_UNLIKELY (ohm_pattern_type_id == 0)) {
		static const GTypeInfo g_define_type_info = {
		  sizeof (OhmPatternClass),
		  (GBaseInitFunc) NULL,
		  (GBaseFinalizeFunc) NULL,
		  (GClassInitFunc) ohm_pattern_class_init,
		  (GClassFinalizeFunc) NULL,
		  NULL,
		  sizeof (OhmPattern),
		  0,
		  (GInstanceInitFunc) ohm_pattern_init
		};

		ohm_pattern_type_id = g_type_register_static (OHM_TYPE_STRUCTURE, "OhmPattern", &g_define_type_info, 0);
	}

	return ohm_pattern_type_id;
}


/**
 * ohm_fact_new:
 * @name: the name of the fact.
 *
 * Create a new #OhmFact with the given @name.
 * A name a well known fact-name like "org.freedesktop.DBus.Object", or "org.alsa-project.mixer".
 *
 * Returns: a new #OhmFact, without any fields.
 **/
OhmFact* ohm_fact_new (const char* name) {
	return g_object_new (OHM_TYPE_FACT, "name", name, NULL);
}


/**
 * ohm_fact_get:
 * @self: a #OhmFact
 * @field: the name of the field to get.
 *
 * Returns: The field value or %NULL if the field does not exist.
 **/
GValue* ohm_fact_get (OhmFact* self, const char* field_name) {
	return ohm_structure_get (OHM_STRUCTURE (self), field_name);
}


/**
 * ohm_fact_set:
 * @self: a #OhmFact
 * @field: the name of the field to set
 * @value: a #GValue with an arbitrary type. If %NULL, then the @field is removed.
 *
 * Set a @field to @value.
 * @value should be allocated by the caller. It will be freed when #OhmFact is destroyed, or the field is removed.
 **/
void ohm_fact_set (OhmFact* self, const char* field_name, GValue* value) {
	g_return_if_fail (OHM_IS_FACT (self));
	g_return_if_fail (field_name != NULL);

	/* fields starting with a double underscore are immutable */
	if (field_name[0] == '_' && field_name[1] == '_' &&
	    ohm_fact_get(self, field_name) != NULL)
	  return;

	/*fixme ?#, fixme array of changes*/
	ohm_structure_set (OHM_STRUCTURE (self), field_name, value);
}


/**
 * ohm_fact_qset:
 * @self: a #OhmFact
 * @field: the #GQuark name of the field to set
 * @value: a #GValue with an arbitrary type. If %NULL, then the @field
 * is removed.
 *
 * Set a @field to @value.
 * @value should be allocated by the caller. It will be freed when
 * #OhmFact is destroyed or the field is removed.
 *
 * When setting a field, the associated :fact_store is notified and
 * collects modification in interested views.  The modification is
 * itself tracked by a #OhmFactStoreTransaction transaction that can
 * cancel and discard the changes, including the view notificiations.
 **/
static void ohm_fact_real_qset (OhmStructure* base, GQuark field, GValue* value) {
	OhmFact * self;

	self = OHM_FACT (base);

	/*fixme ?#
	 save previous value, if any*/
	if (self->priv->_fact_store != NULL) {
		OhmFactStoreTransaction* t;

		t = (OhmFactStoreTransaction*) g_queue_peek_head (self->priv->_fact_store->transaction);
		if (t != NULL) {
			t->modifications = g_slist_prepend (t->modifications,
							    ohm_fact_store_transaction_cow_new (self, OHM_FACT_STORE_EVENT_UPDATED, field, g_object_steal_qdata (G_OBJECT (self), field)));
		}
	}

	OHM_STRUCTURE_CLASS (ohm_fact_parent_class)->qset (OHM_STRUCTURE (self), field, value);

	/* inform the fact_store, and views, if not */
	if (self->priv->_fact_store != NULL) {
		ohm_fact_store_update (ohm_fact_get_fact_store (self), self, field, value);
	}
}


OhmFactStore* ohm_fact_get_fact_store (OhmFact* self) {
	g_return_val_if_fail (OHM_IS_FACT (self), NULL);
	return self->priv->_fact_store;
}


GSList *ohm_fact_get_fields(OhmFact *self) {
	g_return_val_if_fail (OHM_IS_FACT (self), NULL);
	return OHM_STRUCTURE(self)->fields;
}


void ohm_fact_set_fact_store (OhmFact* self, OhmFactStore* value) {
	g_return_if_fail (OHM_IS_FACT (self));


    if (value == NULL && self->priv->_fact_store != NULL)
        g_object_remove_weak_pointer (G_OBJECT (self->priv->_fact_store),
                                      (void *)&self->priv->_fact_store);

	self->priv->_fact_store = value;

	if (self->priv->_fact_store != NULL) {
		g_object_add_weak_pointer (G_OBJECT (self->priv->_fact_store),
					   (void*)&self->priv->_fact_store);
	}
}


static void ohm_fact_get_property (GObject * object, guint property_id, GValue * value, GParamSpec * pspec) {
	OhmFact * self;

	self = OHM_FACT (object);

	switch (property_id) {
	case OHM_FACT_FACT_STORE:
	  g_value_set_object (value, ohm_fact_get_fact_store (self));
	  break;
	default:
	  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	  break;
	}
}


static void ohm_fact_set_property (GObject * object, guint property_id, const GValue * value, GParamSpec * pspec) {
	OhmFact * self;

	self = OHM_FACT (object);

	switch (property_id) {
	case OHM_FACT_FACT_STORE:
	  ohm_fact_set_fact_store (self, g_value_get_object (value));
	  break;
	default:
	  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	  break;
	}
}


static void ohm_fact_class_init (OhmFactClass * klass) {
	ohm_fact_parent_class = g_type_class_peek_parent (klass);

	g_type_class_add_private (klass, sizeof (OhmFactPrivate));

	G_OBJECT_CLASS (klass)->get_property = ohm_fact_get_property;
	G_OBJECT_CLASS (klass)->set_property = ohm_fact_set_property;
	G_OBJECT_CLASS (klass)->dispose = ohm_fact_dispose;

	OHM_STRUCTURE_CLASS (klass)->qset = ohm_fact_real_qset;

	/**
	 * OhmFact:fact_store:
	 *
	 * A weak pointer to the #OhmFactStore this fact belongs to,
	 * if any, otherwise %NULL.
	 **/
	g_object_class_install_property (G_OBJECT_CLASS (klass), OHM_FACT_FACT_STORE,
					 g_param_spec_object ("fact-store", "fact-store", "fact-store", OHM_TYPE_FACT_STORE,
							      G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE));
}


static void ohm_fact_init (OhmFact * self) {
	self->priv = OHM_FACT_GET_PRIVATE (self);
}


static void ohm_fact_dispose (GObject * obj) {
	OhmFact * self;

	self = OHM_FACT (obj);

	if (self->priv->_fact_store != NULL) {
	  g_object_remove_weak_pointer (G_OBJECT (self->priv->_fact_store),
					(gpointer)&self->priv->_fact_store);
	}

	G_OBJECT_CLASS (ohm_fact_parent_class)->dispose (obj);
}


GType ohm_fact_get_type (void) {
	static GType ohm_fact_type_id = 0;

	if (G_UNLIKELY (ohm_fact_type_id == 0)) {
		static const GTypeInfo g_define_type_info = {
		  sizeof (OhmFactClass),
		  (GBaseInitFunc) NULL,
		  (GBaseFinalizeFunc) NULL,
		  (GClassInitFunc) ohm_fact_class_init,
		  (GClassFinalizeFunc) NULL,
		  NULL,
		  sizeof (OhmFact),
		  0,
		  (GInstanceInitFunc) ohm_fact_init
		};

		ohm_fact_type_id = g_type_register_static (OHM_TYPE_STRUCTURE, "OhmFact", &g_define_type_info, 0);
	}

	return ohm_fact_type_id;
}


static void _ohm_fact_store_update_views (OhmFactStore* self, OhmFact* fact, OhmFactStoreEvent event, GQuark field, GValue *value) {
	GSList* patterns;
	GSList* p_collection;
	GSList* p_it;
	OhmFactStoreTransaction* t;

	g_return_if_fail (OHM_IS_FACT_STORE (self));
	g_return_if_fail (OHM_IS_FACT (fact));

#if 0 /* check moved to the caller */
	if (_ohm_fact_store_transaction_rolledback(self) ||
	    _ohm_fact_store_transaction_active(self))
		return;
#endif
	
	t = (OhmFactStoreTransaction*) g_queue_peek_head (self->transaction);

	patterns = g_datalist_id_get_data (&self->priv->interest, ohm_structure_get_qname (OHM_STRUCTURE (fact)));

	p_collection = patterns;
	for (p_it = p_collection; p_it != NULL; p_it = p_it->next) {
	  OhmPatternMatch* m;
	  OhmPattern* p;

	  p = (OhmPattern*) p_it->data;
	  m = ohm_pattern_match (p, fact, event);

	  if (m != NULL) {
	    ohm_fact_store_change_set_add_match (OHM_FACT_STORE_SIMPLE_VIEW (ohm_pattern_get_view (p))->change_set, m);

	    if (t != NULL) {
	      t->matches = g_slist_prepend (t->matches, 
					    ohm_pair_new (g_object_ref (m), 
							  g_object_ref (ohm_pattern_get_view (p)),
							  g_object_unref, g_object_unref));
	    }
	    
	    (m == NULL ? NULL : (m = (g_object_unref (m), NULL)));
	  }
	}

	switch (event) {
	case OHM_FACT_STORE_EVENT_ADDED:
		g_signal_emit_by_name (G_OBJECT (self), "inserted", fact);
		break;
	case OHM_FACT_STORE_EVENT_REMOVED:
		g_signal_emit_by_name (G_OBJECT (self), "removed", fact);
		break;
	case OHM_FACT_STORE_EVENT_UPDATED:
		g_signal_emit_by_name (G_OBJECT (self), "updated", fact, field, value);
		break;
	default:
		break;
	}
}


static gboolean ohm_fact_store_insert_internal (OhmFactStore* self, OhmFact* fact) {
	GSList* facts;

	g_return_val_if_fail (OHM_IS_FACT_STORE (self), FALSE);
	g_return_val_if_fail (OHM_IS_FACT (fact), FALSE);

	facts = ohm_fact_store_get_facts_by_quark (self, ohm_structure_get_qname (OHM_STRUCTURE (fact)));

	if (ohm_fact_get_fact_store (fact) != NULL) {
		return FALSE;
	}

	if (g_slist_find (facts, fact) == NULL) {
		ohm_fact_set_fact_store (fact, self);

		facts = g_slist_prepend (facts, g_object_ref (fact));

		g_object_set_qdata (G_OBJECT (self), ohm_structure_get_qname (OHM_STRUCTURE (fact)), facts);

		return TRUE;
	}
	return FALSE;
}


/**
 * ohm_fact_store_insert:
 * @self: a #OhmFactStore
 * @fact: the fact to be added to the @self fact-store. (not %NULL)
 *
 * Insert a @fact to the @self fact-store. The @fact will be
 * referenced by the fact-store and can hence be unrefed by the
 * caller. The @fact will be unrefed when it is removed or the
 * #OhmFactStore is destroyed.
 *
 * The insertion is reversible if there is a transaction on-going and it
 * is rolled back (for back-tracking purpose, for instance). In that
 * case, the @fact is referenced by the transaction itself, and stay
 * alive until the transaction ends.
 *
 * The views that match the @fact will get notified, unless the
 * transaction is canceled in between.
 *
 * Returns: %TRUE on success.
 **/
gboolean ohm_fact_store_insert (OhmFactStore* self, OhmFact* fact) {
	g_return_val_if_fail (OHM_IS_FACT_STORE (self), FALSE);
	g_return_val_if_fail (OHM_IS_FACT (fact), FALSE);

	if (ohm_fact_store_insert_internal (self, fact)) {
		OhmFactStoreTransaction* t;

		if (g_slist_find (self->priv->known_facts_qname, GINT_TO_POINTER (ohm_structure_get_qname (OHM_STRUCTURE (fact)))) == NULL) {
			self->priv->known_facts_qname = g_slist_prepend (self->priv->known_facts_qname,
									 GINT_TO_POINTER (ohm_structure_get_qname (OHM_STRUCTURE (fact))));
		}

		t = (OhmFactStoreTransaction*) g_queue_peek_head (self->transaction);
		if (t != NULL) {
			t->modifications = g_slist_prepend (t->modifications, ohm_fact_store_transaction_cow_new (fact, OHM_FACT_STORE_EVENT_ADDED, 0, NULL));
		}

	        
		if (!_ohm_fact_store_transaction_rolledback(self) &&
		    _ohm_fact_store_transaction_active(self))
			_ohm_fact_store_update_views (self, fact, OHM_FACT_STORE_EVENT_ADDED, 0, NULL);

		return TRUE;
	}

	return FALSE;
}


static gboolean ohm_fact_store_remove_internal (OhmFactStore* self, OhmFact* fact) {
	GSList* facts;
	GSList* found;

	g_return_val_if_fail (OHM_IS_FACT_STORE (self), FALSE);
	g_return_val_if_fail (OHM_IS_FACT (fact), FALSE);

	facts = ohm_fact_store_get_facts_by_quark (self, ohm_structure_get_qname (OHM_STRUCTURE (fact)));
	found = g_slist_find (facts, fact);

	if (found != NULL) {
		facts = g_slist_delete_link (facts, found);
		g_object_set_qdata (G_OBJECT (self), ohm_structure_get_qname (OHM_STRUCTURE (fact)), facts);
		ohm_fact_set_fact_store (fact, NULL);
		g_object_unref (G_OBJECT (fact));

		return TRUE;
	}

	return FALSE;
}


/**
 * ohm_fact_store_remove:
 * @self: a #OhmFactStore
 * @fact: the fact to be removed from @self fact-store. (not %NULL)
 *
 * Remove a @fact from the @self fact-store. The @fact will be
 * un-referenced.
 *
 * The removal is reversible if there is a transaction on-going and it
 * is rolled back (for back-tracking purpose, for instance). In that
 * case, the @fact is referenced by the transaction itself, and stay
 * alive until the transaction ends.
 *
 * The views that match the @fact will be notified, unless the
 * transaction is canceled in between.
 **/
void ohm_fact_store_remove (OhmFactStore* self, OhmFact* fact) {
	g_return_if_fail (OHM_IS_FACT_STORE (self));
	g_return_if_fail (OHM_IS_FACT (fact));

	if (ohm_fact_store_remove_internal (self, fact)) {
		OhmFactStoreTransaction* t;

		t = (OhmFactStoreTransaction*) g_queue_peek_head (self->transaction);
		if (t != NULL) {
			t->modifications = g_slist_prepend (t->modifications,
							    ohm_fact_store_transaction_cow_new (fact, OHM_FACT_STORE_EVENT_REMOVED, 0, NULL));
		}

		if (!_ohm_fact_store_transaction_rolledback(self) &&
		    _ohm_fact_store_transaction_active(self))
			_ohm_fact_store_update_views (self, fact, OHM_FACT_STORE_EVENT_REMOVED, 0, NULL);
	}
}


/**
 * ohm_fact_store_update:
 * @self: a #OhmFactStore
 * @fact: the fact to be updated. (not %NULL)
 *
 * <note>
 * <para>
 * This function is used internally. You should use ohm_fact_set ()
 * functions if you modify a fact.
 * </para>
 * </note>
 *
 * Update a @fact from the @self fact-store.
 *
 * The update is reversible if there is a transaction on-going and it
 * is rolled back (for back-tracking purpose, for instance). In that
 * case, the modification is saved by the transaction.
 *
 * The views that match the @fact will be notified, unless the
 * transaction is canceled in between.
 **/
void ohm_fact_store_update (OhmFactStore* self, OhmFact* fact, GQuark field, GValue* value) {
	g_return_if_fail (OHM_IS_FACT_STORE (self));
	g_return_if_fail (OHM_IS_FACT (fact));

	if (!_ohm_fact_store_transaction_rolledback(self) &&
	    _ohm_fact_store_transaction_active(self))
		_ohm_fact_store_update_views (self, fact, OHM_FACT_STORE_EVENT_UPDATED, field, value);
}


/**
 * ohm_fact_store_get_facts_by_quark:
 * @self: a #OhmFactStore
 * @qname: #GQuark name of the facts to list
 *
 * This function is used mostly for debugging purposes. Better use a
 * #OhmFactStoreView, for example, although you might need it.
 *
 * Returns: a weak list of weak #OhmFact that have the
 * name @qname. The caller should not free, modify or unref anything.
 **/
GSList* ohm_fact_store_get_facts_by_quark (OhmFactStore* self, GQuark qname) {
	g_return_val_if_fail (OHM_IS_FACT_STORE (self), NULL);

	return ((GSList*) g_object_get_qdata (G_OBJECT (self), qname));
}


/**
 * ohm_fact_store_get_facts_by_name:
 * @self: a #OhmFactStore
 * @name: name of the facts to list
 *
 * String version of ohm_fact_store_get_facts_by_quark ().
 *
 * Returns: a weak list of weak #OhmFact.
 **/
GSList* ohm_fact_store_get_facts_by_name (OhmFactStore* self, const char* name) {
	g_return_val_if_fail (OHM_IS_FACT_STORE (self), NULL);
	g_return_val_if_fail (name != NULL, NULL);

	return ((GSList*) g_object_get_data (G_OBJECT (self), name));
}


static void _g_slist_free_g_object_unref (GSList* self) {
	g_slist_foreach (self, ((GFunc) g_object_unref), NULL);
	g_slist_free (self);
}


/**
 * ohm_fact_store_get_facts_by_pattern:
 * @self: a #OhmFactStore
 * @pattern: a @pattern (not %NULL)
 *
 * Get the list of facts that match the #OhmPattern @pattern.
 *
 * Returns: a new list of #OhmFact. The caller is responsible to unref
 * elements and free the list.
 **/
GSList* ohm_fact_store_get_facts_by_pattern (OhmFactStore* self, OhmPattern* pattern) {
	GSList* facts;
	GSList* result;
	GSList* f_it;

	g_return_val_if_fail (OHM_IS_FACT_STORE (self), NULL);
	g_return_val_if_fail (OHM_IS_PATTERN (pattern), NULL);

	facts = ((GSList*) g_object_get_qdata (G_OBJECT (self), ohm_structure_get_qname (OHM_STRUCTURE (pattern))));

	result = NULL;

	for (f_it = facts; f_it != NULL; f_it = f_it->next) {
	  OhmFact* f;
	  OhmPatternMatch* m;

	  f = g_object_ref (f_it->data);
	  m = ohm_pattern_match (pattern, f, OHM_FACT_STORE_EVENT_LOOKUP);

	  if (m != NULL) {
	    result = g_slist_prepend (result, m);
	    m = NULL;
	  }

	  g_object_unref (f);
	}

	return result;
}


/**
 * ohm_fact_store_transaction_push:
 * @self: a #OhmFactStore
 *
 * Start a new transaction (on top of the previous).
 **/
void ohm_fact_store_transaction_push (OhmFactStore* self) {
	OhmFactStoreTransaction* trans;

	g_return_if_fail (OHM_IS_FACT_STORE (self));

	trans = ohm_fact_store_transaction_new (self, G_OBJECT (self));

	g_queue_push_head (self->transaction, trans);
}


/**
 * ohm_fact_store_transaction_pop:
 * @self: a #OhmFactStore
 * @rollback: wether to roll-back the transaction (%FALSE if not)
 *
 * Finish the top transaction and restore to the previous transaction state.
 **/
void ohm_fact_store_transaction_pop (OhmFactStore* self, gboolean rollback) {
	OhmFactStoreTransaction* trans;
	OhmFactStoreTransaction* _tmp3;

	g_return_if_fail (OHM_IS_FACT_STORE (self));

	trans = ((OhmFactStoreTransaction*) g_queue_pop_head (self->transaction));
	g_queue_push_head (self->transaction, NULL);

	/* just to lock any transaction system*/
	if (trans != NULL) {
		GSList* p_collection;
		GSList* p_it;
		GSList* cow_collection;
		GSList* cow_it;
		
		if (rollback) {
			p_collection = trans->matches;
			for (p_it = p_collection; p_it != NULL; p_it = p_it->next) {
				OhmPair* p;
				OhmPatternMatch* m;
				OhmFactStoreView* v;
				gboolean          warned = FALSE;

				p = ((OhmPair*) p_it->data);
				
				m = (OhmPatternMatch*) p->first;
				v = (OhmFactStoreView*) p->second;
				ohm_fact_store_change_set_remove_match (OHM_FACT_STORE_SIMPLE_VIEW (v)->change_set, m);

				if (!warned) {
					g_warning("Hmm... transaction rollback with non-empty matches!");
					warned = TRUE;
				}
			}
			
			cow_collection = trans->modifications;
			for (cow_it = cow_collection; cow_it != NULL; cow_it = cow_it->next) {
				OhmFactStoreTransactionCOW* cow;

				cow = (OhmFactStoreTransactionCOW*) cow_it->data;

				switch (cow->event) {
				case OHM_FACT_STORE_EVENT_ADDED: {
				  ohm_fact_store_remove_internal (self, cow->fact);
				  break;
				}
				case OHM_FACT_STORE_EVENT_REMOVED: {
				  ohm_fact_store_insert_internal (self, cow->fact);
				  break;
				}
				case OHM_FACT_STORE_EVENT_UPDATED: {
				  ohm_structure_qset (OHM_STRUCTURE (cow->fact), cow->field, cow->value);
				  cow->value = NULL;
				  break;
				}
				case OHM_FACT_STORE_EVENT_LOOKUP: {
				  g_warning ("lookup should not happen");
				  break;
				}
				default:
				  break;
				}
			}
		}
		else {
			cow_collection = trans->modifications;
			for (cow_it = cow_collection; cow_it != NULL; cow_it = cow_it->next) {
				OhmFactStoreTransactionCOW* cow;

				cow = (OhmFactStoreTransactionCOW*) cow_it->data;

				switch (cow->event) {
				case OHM_FACT_STORE_EVENT_ADDED:
					_ohm_fact_store_update_views(self, cow->fact, OHM_FACT_STORE_EVENT_ADDED, 0, NULL);
					break;
				case OHM_FACT_STORE_EVENT_REMOVED:
					_ohm_fact_store_update_views(self, cow->fact, OHM_FACT_STORE_EVENT_REMOVED, 0, NULL);
					break;
				case OHM_FACT_STORE_EVENT_UPDATED:
					/* XXX TODO Ideally we should store the fact field and value for
					   emitting the full-blown "updated" factstore gobject signal */
					_ohm_fact_store_update_views(self, cow->fact, OHM_FACT_STORE_EVENT_UPDATED, 0, NULL);
					break;
				default:
					break;
				}
			}
		}
	}
	
	_tmp3 = NULL;
	_tmp3 = ((OhmFactStoreTransaction*) g_queue_pop_head (self->transaction));
	(_tmp3 == NULL ? NULL : (_tmp3 = (g_object_unref (_tmp3), NULL)));
	(trans == NULL ? NULL : (trans = (g_object_unref (trans), NULL)));
}


static gboolean _ohm_fact_store_transaction_active(OhmFactStore *self) {
	return (g_queue_peek_head (self->transaction) != NULL);
}


static gboolean _ohm_fact_store_transaction_rolledback(OhmFactStore *self) {
	return (g_queue_peek_head (self->transaction) == NULL &&
		g_queue_get_length (self->transaction) != 0);
}


/* remove the transaction lock
*
 * ohm_fact_store_new:
 *
 * Use ohm_get_fact_store () instead, to get the Ohm single one.
 * Create a new #OhmFactStore.
 *
 * Returns: a new #OhmFactStore
 **/
OhmFactStore* ohm_fact_store_new (void) {
	return g_object_newv (OHM_TYPE_FACT_STORE, 0, NULL);
}


/**
 * ohm_fact_store_to_string:
 * @self: a #OhmFactStore
 *
 * Return a debug representation of the @self fact-store. (From vala version)
 *
 * Returns: a new string to be free by the caller.
 **/
char* ohm_fact_store_to_string (OhmFactStore* self) {
	GSList* q_collection;
	GSList* q_it;
	char* ret;

	g_return_val_if_fail (OHM_IS_FACT_STORE (self), NULL);

	ret = g_strdup_printf ("FactStore %p:\n\n", self);
	q_collection = self->priv->known_facts_qname;
	for (q_it = q_collection; q_it != NULL; q_it = q_it->next) {
	  GQuark q;
	  q = GPOINTER_TO_INT (q_it->data);
	  {
	    {
	      GSList* f_collection;
	      GSList* f_it;
	      f_collection = ohm_fact_store_get_facts_by_quark (self, q);
	      for (f_it = f_collection; f_it != NULL; f_it = f_it->next) {
		OhmFact* _tmp3;
		OhmFact* f;
		_tmp3 = NULL;
		f = (_tmp3 = ((OhmFact*) f_it->data), (_tmp3 == NULL ? NULL : g_object_ref (_tmp3)));
		{
		  char* _tmp1;
		  char* _tmp0;
		  char* _tmp2;
		  _tmp1 = NULL;
		  _tmp0 = NULL;
		  ret = (_tmp1 = g_strconcat (ret, (_tmp0 = (ohm_structure_to_string (OHM_STRUCTURE (f)))), NULL), (ret = (g_free (ret), NULL)), _tmp1);
		  _tmp0 = (g_free (_tmp0), NULL);
		  _tmp2 = NULL;
		  ret = (_tmp2 = g_strconcat (ret, ("\n"), NULL), (ret = (g_free (ret), NULL)), _tmp2);
		  (f == NULL ? NULL : (f = (g_object_unref (f), NULL)));
		}
	      }
	    }
	  }
	}

	return ret;
}


/**
 * ohm_fact_store_new_view:
 * @self: a #OhmFactStore
 * @listener: the caller or the owner of the #OhmView (for debugging purpose)
 *
 * Create a view for the fact-store @self. See #OhmFactStoreView description.
 *
 * Returns: a new #OhmFactStoreView
 **/
OhmFactStoreView* ohm_fact_store_new_view (OhmFactStore* self, GObject* listener) {
	g_return_val_if_fail (OHM_IS_FACT_STORE (self), NULL);
	g_return_val_if_fail (listener == NULL || G_IS_OBJECT (listener), NULL);

	return ohm_fact_store_view_new (self, listener);
}


static void _ohm_fact_store_delete_func (GSList* l) {
	GSList* p_collection;
	GSList* p_it;
	OhmPattern* p;

	p_collection = l;
	for (p_it = p_collection; p_it != NULL; p_it = p_it->next) {
	  p = (OhmPattern*) p_it->data;
	  g_object_unref (G_OBJECT (p));
	}

	g_slist_free (l);
}


static void ohm_fact_store_set_view_interest (OhmFactStore* self, OhmFactStoreView* v) {
	GSList* p_collection;
	GSList* p_it;
	GSList* patterns;
	GSList* patts;

	g_return_if_fail (OHM_IS_FACT_STORE (self));
	g_return_if_fail (OHM_FACT_STORE_IS_VIEW (v));

	p_collection = v->patterns;
	for (p_it = p_collection; p_it != NULL; p_it = p_it->next) {
	  OhmPattern* p;

	  p = (OhmPattern*) p_it->data;
	  patterns = g_datalist_id_remove_no_notify (&self->priv->interest, ohm_structure_get_qname (OHM_STRUCTURE (p)));
	  patts = patterns;

	  /* FIXME...*/
	  if (g_slist_find (((gpointer) patterns), p) == NULL) {

	    if (ohm_pattern_get_view (p) != NULL) {
	      g_warning ("%p: pattern has already a view", p);
	    }

	    ohm_pattern_set_view (p, v);
	    patts = g_slist_prepend (patts, g_object_ref (p));
	    /* FIXME: match now?*/
	    patterns = patts;
	  }

	  if (patterns != NULL) {
	    g_datalist_id_set_data_full (&self->priv->interest, ohm_structure_get_qname (OHM_STRUCTURE (p)), patterns, ((GDestroyNotify) _ohm_fact_store_delete_func));
	  }
	}
}


static void ohm_fact_store_pattern_del (OhmFactStore* self, OhmFactStoreView* v, OhmStructure *interest) {
	GSList *patterns;
	GQuark  id;
	OhmPattern *p;

	g_return_if_fail (OHM_IS_FACT_STORE (self));
	g_return_if_fail (OHM_FACT_STORE_IS_VIEW (v));
	g_return_if_fail (OHM_IS_PATTERN(interest));

	id = ohm_structure_get_qname (interest);
	patterns = g_datalist_id_remove_no_notify (&self->priv->interest, id);
	p = OHM_PATTERN(interest);
	
	if (g_slist_index(patterns, p) < 0)
		return;
	
	if ((patterns = g_slist_remove(patterns, p)) != NULL)
		g_datalist_id_set_data_full (&self->priv->interest, id, patterns, ((GDestroyNotify) _ohm_fact_store_delete_func));
}


void ohm_fact_store_change_set_add_match (OhmFactStoreChangeSet* self, OhmPatternMatch* match) {
	g_return_if_fail (OHM_FACT_STORE_IS_CHANGE_SET (self));
	g_return_if_fail (OHM_PATTERN_IS_MATCH (match));

	self->priv->_matches = g_slist_prepend (self->priv->_matches, g_object_ref (match));
}


void ohm_fact_store_change_set_remove_match (OhmFactStoreChangeSet* self, OhmPatternMatch* match) {
	g_return_if_fail (OHM_FACT_STORE_IS_CHANGE_SET (self));
	g_return_if_fail (OHM_PATTERN_IS_MATCH (match));

	self->priv->_matches = g_slist_remove (self->priv->_matches, match);
}


void ohm_fact_store_change_set_reset (OhmFactStoreChangeSet* self) {
	g_return_if_fail (OHM_FACT_STORE_IS_CHANGE_SET (self));

	if (self->priv->_matches != NULL) {
	  _g_slist_free_g_object_unref (self->priv->_matches);
	}

	self->priv->_matches = NULL;
}


char* ohm_fact_store_change_set_to_string (OhmFactStoreChangeSet* self) {
	g_return_val_if_fail (OHM_FACT_STORE_IS_CHANGE_SET (self), NULL);

	return g_strdup_printf ("n matches: %d", g_slist_length (ohm_fact_store_change_set_get_matches (self)));
}


/**
 * OhmFactStoreChangeSet:
 *
 * This object serves as an aggregator of the changes.
 *
 **/
OhmFactStoreChangeSet* ohm_fact_store_change_set_new (void) {
	OhmFactStoreChangeSet * self;

	self = g_object_newv (OHM_FACT_STORE_TYPE_CHANGE_SET, 0, NULL);

	return self;
}


GSList* ohm_fact_store_change_set_get_matches (OhmFactStoreChangeSet* self) {
	g_return_val_if_fail (OHM_FACT_STORE_IS_CHANGE_SET (self), NULL);

	return self->priv->_matches;
}


static void ohm_fact_store_change_set_get_property (GObject * object, guint property_id, GValue * value, GParamSpec * pspec) {
	OhmFactStoreChangeSet * self;

	self = OHM_FACT_STORE_CHANGE_SET (object);

	switch (property_id) {
	case OHM_FACT_STORE_CHANGE_SET_MATCHES:
	  g_value_set_pointer (value, ohm_fact_store_change_set_get_matches (self));
	  break;
	default:
	  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	  break;
	}
}


static void ohm_fact_store_change_set_class_init (OhmFactStoreChangeSetClass * klass) {
	ohm_fact_store_change_set_parent_class = g_type_class_peek_parent (klass);

	g_type_class_add_private (klass, sizeof (OhmFactStoreChangeSetPrivate));

	G_OBJECT_CLASS (klass)->get_property = ohm_fact_store_change_set_get_property;
	G_OBJECT_CLASS (klass)->dispose = ohm_fact_store_change_set_dispose;

	g_object_class_install_property (G_OBJECT_CLASS (klass), OHM_FACT_STORE_CHANGE_SET_MATCHES,
					 g_param_spec_pointer ("matches", "matches", "matches",
							       G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE));
}


static void ohm_fact_store_change_set_init (OhmFactStoreChangeSet * self) {
	self->priv = OHM_FACT_STORE_CHANGE_SET_GET_PRIVATE (self);
}


static void ohm_fact_store_change_set_dispose (GObject * obj) {
	OhmFactStoreChangeSet * self;

	self = OHM_FACT_STORE_CHANGE_SET (obj);

	if (self->priv->_matches != NULL) {
	  _g_slist_free_g_object_unref (self->priv->_matches);
	  self->priv->_matches = NULL;
	}

	G_OBJECT_CLASS (ohm_fact_store_change_set_parent_class)->dispose (obj);
}


GType ohm_fact_store_change_set_get_type (void) {
	static GType ohm_fact_store_change_set_type_id = 0;

	if (G_UNLIKELY (ohm_fact_store_change_set_type_id == 0)) {
	  static const GTypeInfo g_define_type_info = {
	    sizeof (OhmFactStoreChangeSetClass),
	    (GBaseInitFunc) NULL,
	    (GBaseFinalizeFunc) NULL,
	    (GClassInitFunc) ohm_fact_store_change_set_class_init,
	    (GClassFinalizeFunc) NULL,
	    NULL,
	    sizeof (OhmFactStoreChangeSet),
	    0,
	    (GInstanceInitFunc) ohm_fact_store_change_set_init
	  };

	  ohm_fact_store_change_set_type_id = g_type_register_static (G_TYPE_OBJECT, "OhmFactStoreChangeSet", &g_define_type_info, 0);
	}

	return ohm_fact_store_change_set_type_id;
}


/**
 * OhmFactStoreSimpleView:
 *
 * The base class to implement views.
 * You typically need #OhmFactStoreView only.
 *
 **/
OhmFactStoreSimpleView* ohm_fact_store_simple_view_new (void) {
	OhmFactStoreSimpleView * self;

	self = g_object_newv (OHM_FACT_STORE_TYPE_SIMPLE_VIEW, 0, NULL);

	return self;
}


GObject* ohm_fact_store_simple_view_get_listener (OhmFactStoreSimpleView* self) {
	g_return_val_if_fail (OHM_FACT_STORE_IS_SIMPLE_VIEW (self), NULL);

	return self->priv->_listener;
}


void ohm_fact_store_simple_view_set_listener (OhmFactStoreSimpleView* self, GObject* value) {
	g_return_if_fail (OHM_FACT_STORE_IS_SIMPLE_VIEW (self));

	self->priv->_listener = value;
}


OhmFactStore* ohm_fact_store_simple_view_get_fact_store (OhmFactStoreSimpleView* self) {
	g_return_val_if_fail (OHM_FACT_STORE_IS_SIMPLE_VIEW (self), NULL);

	return self->priv->_fact_store;
}


void ohm_fact_store_simple_view_set_fact_store (OhmFactStoreSimpleView* self, OhmFactStore* value) {
	g_return_if_fail (OHM_FACT_STORE_IS_SIMPLE_VIEW (self));

	self->priv->_fact_store = value;

	g_object_add_weak_pointer (G_OBJECT (self->priv->_fact_store), (void*)&self->priv->_fact_store);
}


static GObject * ohm_fact_store_simple_view_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	OhmFactStoreSimpleViewClass * klass;
	GObjectClass * parent_class;
	OhmFactStoreSimpleView * self;

	klass = OHM_FACT_STORE_SIMPLE_VIEW_CLASS (g_type_class_peek (OHM_FACT_STORE_TYPE_SIMPLE_VIEW));
	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);

	self = OHM_FACT_STORE_SIMPLE_VIEW (obj);
	self->change_set = ohm_fact_store_change_set_new ();

	return obj;
}


static void ohm_fact_store_simple_view_get_property (GObject * object, guint property_id, GValue * value, GParamSpec * pspec) {
	OhmFactStoreSimpleView * self;

	self = OHM_FACT_STORE_SIMPLE_VIEW (object);

	switch (property_id) {
	case OHM_FACT_STORE_SIMPLE_VIEW_LISTENER:
	  g_value_set_object (value, ohm_fact_store_simple_view_get_listener (self));
	  break;
	case OHM_FACT_STORE_SIMPLE_VIEW_FACT_STORE:
	  g_value_set_object (value, ohm_fact_store_simple_view_get_fact_store (self));
	  break;
	default:
	  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	  break;
	}
}


static void ohm_fact_store_simple_view_set_property (GObject * object, guint property_id, const GValue * value, GParamSpec * pspec) {
	OhmFactStoreSimpleView * self;

	self = OHM_FACT_STORE_SIMPLE_VIEW (object);

	switch (property_id) {
	case OHM_FACT_STORE_SIMPLE_VIEW_LISTENER:
	  ohm_fact_store_simple_view_set_listener (self, g_value_get_object (value));
	  break;
	case OHM_FACT_STORE_SIMPLE_VIEW_FACT_STORE:
	  ohm_fact_store_simple_view_set_fact_store (self, g_value_get_object (value));
	  break;
	default:
	  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	  break;
	}
}


static void ohm_fact_store_simple_view_class_init (OhmFactStoreSimpleViewClass * klass) {
	ohm_fact_store_simple_view_parent_class = g_type_class_peek_parent (klass);

	g_type_class_add_private (klass, sizeof (OhmFactStoreSimpleViewPrivate));

	G_OBJECT_CLASS (klass)->get_property = ohm_fact_store_simple_view_get_property;
	G_OBJECT_CLASS (klass)->set_property = ohm_fact_store_simple_view_set_property;
	G_OBJECT_CLASS (klass)->constructor = ohm_fact_store_simple_view_constructor;
	G_OBJECT_CLASS (klass)->dispose = ohm_fact_store_simple_view_dispose;

	g_object_class_install_property (G_OBJECT_CLASS (klass), OHM_FACT_STORE_SIMPLE_VIEW_LISTENER,
					 g_param_spec_object ("listener", "listener", "listener", G_TYPE_OBJECT,
							      G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (G_OBJECT_CLASS (klass), OHM_FACT_STORE_SIMPLE_VIEW_FACT_STORE,
					 g_param_spec_object ("fact-store", "fact-store", "fact-store", OHM_TYPE_FACT_STORE,
							      G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

	g_signal_new ("updated", OHM_FACT_STORE_TYPE_SIMPLE_VIEW, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, OHM_FACT_STORE_TYPE_CHANGE_SET);
}


static void ohm_fact_store_simple_view_init (OhmFactStoreSimpleView * self) {
	self->priv = OHM_FACT_STORE_SIMPLE_VIEW_GET_PRIVATE (self);

	self->priv->_listener = NULL;
}


static void ohm_fact_store_simple_view_dispose (GObject * obj) {
	OhmFactStoreSimpleView * self;

	self = OHM_FACT_STORE_SIMPLE_VIEW (obj);

	if (self->priv->_fact_store != NULL) {
	  g_object_remove_weak_pointer (G_OBJECT (self->priv->_fact_store),
					(gpointer)&self->priv->_fact_store);
	}

	if (self->change_set != NULL) {
	  g_object_unref (self->change_set);
	  self->change_set = NULL;
	}

	G_OBJECT_CLASS (ohm_fact_store_simple_view_parent_class)->dispose (obj);
}


GType ohm_fact_store_simple_view_get_type (void) {
	static GType ohm_fact_store_simple_view_type_id = 0;

	if (G_UNLIKELY (ohm_fact_store_simple_view_type_id == 0)) {
		static const GTypeInfo g_define_type_info = {
		  sizeof (OhmFactStoreSimpleViewClass),
		  (GBaseInitFunc) NULL,
		  (GBaseFinalizeFunc) NULL,
		  (GClassInitFunc) ohm_fact_store_simple_view_class_init,
		  (GClassFinalizeFunc) NULL,
		  NULL,
		  sizeof (OhmFactStoreSimpleView),
		  0,
		  (GInstanceInitFunc) ohm_fact_store_simple_view_init
		};

		ohm_fact_store_simple_view_type_id = g_type_register_static (G_TYPE_OBJECT, "OhmFactStoreSimpleView", &g_define_type_info, 0);
	}

	return ohm_fact_store_simple_view_type_id;
}


static void _g_slist_free_ohm_pair_free (GSList* self) {
	g_slist_foreach (self, ((GFunc) ohm_pair_free), NULL);

	g_slist_free (self);
}


static void _g_slist_free_ohm_fact_store_transaction_cow_free (GSList* self) {
	g_slist_foreach (self, ((GFunc) ohm_fact_store_transaction_cow_free), NULL);

	g_slist_free (self);
}


static OhmFactStoreTransaction* ohm_fact_store_transaction_new (OhmFactStore* fact_store, GObject* listener) {
	g_return_val_if_fail (OHM_IS_FACT_STORE (fact_store), NULL);
	g_return_val_if_fail (G_IS_OBJECT (listener), NULL);

	return g_object_new (OHM_FACT_STORE_TYPE_TRANSACTION,
			     "fact-store", fact_store,
			     "listener", listener,
			     NULL);
}


OhmFactStoreTransactionCOW* ohm_fact_store_transaction_cow_new (OhmFact* fact, OhmFactStoreEvent event, GQuark field, GValue* value) {
	OhmFactStoreTransactionCOW* self;

	g_return_val_if_fail (OHM_IS_FACT (fact), NULL);

	self = g_slice_new0 (OhmFactStoreTransactionCOW);

	self->fact = g_object_ref (fact);
	self->event = (gint) event;
	self->field = field;
	self->value = value;

	return self;
}


void ohm_fact_store_transaction_cow_free (OhmFactStoreTransactionCOW* self) {
	if (self->fact != NULL) {
	  g_object_unref (self->fact);
	  self->fact = NULL;
	}
	
	if (self->value != NULL) {
	  g_value_unset(self->value);
	  g_free(self->value);
	}

	g_slice_free (OhmFactStoreTransactionCOW, self);
}


static void ohm_fact_store_transaction_class_init (OhmFactStoreTransactionClass * klass) {
	ohm_fact_store_transaction_parent_class = g_type_class_peek_parent (klass);

	G_OBJECT_CLASS (klass)->dispose = ohm_fact_store_transaction_dispose;
}


static void ohm_fact_store_transaction_init (OhmFactStoreTransaction * self) {
}


static void ohm_fact_store_transaction_dispose (GObject * obj) {
	OhmFactStoreTransaction * self;

	self = OHM_FACT_STORE_TRANSACTION (obj);

	if (self->matches != NULL) {
	  _g_slist_free_ohm_pair_free (self->matches);
	  self->matches = NULL;
	}

	if (self->modifications != NULL) {
	  _g_slist_free_ohm_fact_store_transaction_cow_free (self->modifications);
	  self->modifications = NULL;
	}

	G_OBJECT_CLASS (ohm_fact_store_transaction_parent_class)->dispose (obj);
}


GType ohm_fact_store_transaction_get_type (void) {
	static GType ohm_fact_store_transaction_type_id = 0;

	if (G_UNLIKELY (ohm_fact_store_transaction_type_id == 0)) {
		static const GTypeInfo g_define_type_info = {
		  sizeof (OhmFactStoreTransactionClass),
		  (GBaseInitFunc) NULL,
		  (GBaseFinalizeFunc) NULL,
		  (GClassInitFunc) ohm_fact_store_transaction_class_init,
		  (GClassFinalizeFunc) NULL,
		  NULL,
		  sizeof (OhmFactStoreTransaction),
		  0,
		  (GInstanceInitFunc) ohm_fact_store_transaction_init
		};

		ohm_fact_store_transaction_type_id = g_type_register_static (OHM_FACT_STORE_TYPE_SIMPLE_VIEW, "OhmFactStoreTransaction", &g_define_type_info, 0);
	}

	return ohm_fact_store_transaction_type_id;
}


static OhmFactStoreView* ohm_fact_store_view_new (OhmFactStore* fact_store, GObject* listener) {
	g_return_val_if_fail (OHM_IS_FACT_STORE (fact_store), NULL);
	g_return_val_if_fail (listener == NULL || G_IS_OBJECT (listener), NULL);

	return g_object_new (OHM_FACT_STORE_TYPE_VIEW,
			     "fact-store", fact_store,
			     "listener", listener,
			     NULL);
}


/**
 * ohm_fact_store_view_add:
 * @self: a #OhmFactStoreView
 * @interest: a #OhmFact or a #OhmPattern (not #NULL)
 *
 * Add a fact or a pattern to the view interest
 **/
void ohm_fact_store_view_add (OhmFactStoreView* self, OhmStructure* interest) {
	g_return_if_fail (OHM_FACT_STORE_IS_VIEW (self));
	g_return_if_fail (OHM_IS_STRUCTURE (interest));

	if (OHM_IS_FACT (interest)) {
		self->patterns = g_slist_prepend (self->patterns, ohm_pattern_new_for_fact (OHM_FACT (interest)));
	}

	if (OHM_IS_PATTERN (interest)) {
		if (g_slist_find (self->patterns, interest) == NULL) {
			g_object_ref (interest);
			self->patterns = g_slist_prepend (self->patterns, interest);
		}
	}

	ohm_fact_store_set_view_interest (ohm_fact_store_simple_view_get_fact_store (OHM_FACT_STORE_SIMPLE_VIEW (self)), self);
}


/**
 * ohm_fact_store_view_remove:
 * @self: a #OhmFactStoreView
 * @interest: a #OhmFact or a #OhmPattern (not NULL)
 *
 * Remove a fact or a pattern from the view interest.
 **/
void ohm_fact_store_view_remove (OhmFactStoreView* self, OhmStructure* interest) {
	g_return_if_fail (OHM_FACT_STORE_IS_VIEW (self));
	g_return_if_fail (OHM_IS_STRUCTURE (interest));

	if (OHM_IS_FACT (interest)) {
		g_message ("not implemented");
		return;
	}

	if (OHM_IS_PATTERN (interest)) {
	  	g_object_unref (interest);
		self->patterns = g_slist_remove_all (self->patterns, interest);
	}

	ohm_fact_store_pattern_del(ohm_fact_store_simple_view_get_fact_store (OHM_FACT_STORE_SIMPLE_VIEW (self)), self, interest);
}


/**
 * ohm_fact_store_view_to_string:
 * @self: a #OhmFactStoreView
 *
 * Returns: a newly allocated string to debug the fact store.
 **/
char* ohm_fact_store_view_to_string (OhmFactStoreView* self) {
	char* ret;

	g_return_val_if_fail (OHM_FACT_STORE_IS_VIEW (self), NULL);

	ret = g_strdup_printf ("listener: %p, factstore: %p, patterns: %d, changeset: %s",
			       ohm_fact_store_simple_view_get_listener (OHM_FACT_STORE_SIMPLE_VIEW (self)),
			       ohm_fact_store_simple_view_get_fact_store (OHM_FACT_STORE_SIMPLE_VIEW (self)),
			       g_slist_length (self->patterns),
			       ohm_fact_store_change_set_to_string (OHM_FACT_STORE_SIMPLE_VIEW (self)->change_set));

	return ret;
}


static void ohm_fact_store_view_class_init (OhmFactStoreViewClass * klass) {
	ohm_fact_store_view_parent_class = g_type_class_peek_parent (klass);

	G_OBJECT_CLASS (klass)->dispose = ohm_fact_store_view_dispose;
}


static void ohm_fact_store_view_init (OhmFactStoreView * self) {
}


static void ohm_fact_store_view_dispose (GObject * obj) {
	OhmFactStoreView * self;

	self = OHM_FACT_STORE_VIEW (obj);

	if (self->patterns != NULL) {
	  _g_slist_free_g_object_unref (self->patterns);
	  self->patterns = NULL;
	}

	G_OBJECT_CLASS (ohm_fact_store_view_parent_class)->dispose (obj);
}


GType ohm_fact_store_view_get_type (void) {
	static GType ohm_fact_store_view_type_id = 0;

	if (G_UNLIKELY (ohm_fact_store_view_type_id == 0)) {
		static const GTypeInfo g_define_type_info = {
		  sizeof (OhmFactStoreViewClass),
		  (GBaseInitFunc) NULL,
		  (GBaseFinalizeFunc) NULL,
		  (GClassInitFunc) ohm_fact_store_view_class_init,
		  (GClassFinalizeFunc) NULL,
		  NULL,
		  sizeof (OhmFactStoreView),
		  0,
		  (GInstanceInitFunc) ohm_fact_store_view_init
		};

		ohm_fact_store_view_type_id = g_type_register_static (OHM_FACT_STORE_TYPE_SIMPLE_VIEW, "OhmFactStoreView", &g_define_type_info, 0);
	}

	return ohm_fact_store_view_type_id;
}



GType ohm_fact_store_event_get_type (void) {
	static GType ohm_fact_store_event_type_id = 0;

	if (G_UNLIKELY (ohm_fact_store_event_type_id == 0)) {
		static const GEnumValue values[] = {
		  { OHM_FACT_STORE_EVENT_ADDED, "OHM_FACT_STORE_EVENT_ADDED", "added" },
		  { OHM_FACT_STORE_EVENT_REMOVED, "OHM_FACT_STORE_EVENT_REMOVED", "removed" },
		  { OHM_FACT_STORE_EVENT_UPDATED, "OHM_FACT_STORE_EVENT_UPDATED", "updated" },
		  { OHM_FACT_STORE_EVENT_LOOKUP, "OHM_FACT_STORE_EVENT_LOOKUP", "lookup" },
		  { 0, NULL, NULL }
		};

		ohm_fact_store_event_type_id = g_enum_register_static ("OhmFactStoreEvent", values);
	}

	return ohm_fact_store_event_type_id;
}


static void ohm_fact_store_class_init (OhmFactStoreClass * klass) {
	ohm_fact_store_parent_class = g_type_class_peek_parent (klass);

	g_type_class_add_private (klass, sizeof (OhmFactStorePrivate));

	G_OBJECT_CLASS (klass)->dispose = ohm_fact_store_dispose;

	/**
	 * OhmFactStore::inserted:
	 * @arg1: the inserted fact.
	 *
	 * Emits ::inserted when a #OhmFact is added to @ohmfactstore.
	 * Use it only for special purpose, like
	 * debugging. #OhmFactStoreView is way better, with the proper
	 * transaction support.
	 * <note>
	 * <para>
	 * This signal might be removed in the future.
	 * </para>
	 * </note>
	 **/
	g_signal_new ("inserted", OHM_TYPE_FACT_STORE, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, OHM_TYPE_FACT);
	/**
	 * OhmFactStore::removed:
	 * @arg1: the removed fact.
	 *
	 * Emits ::removed when a #OhmFact is removed from @ohmfactstore.
	 * Use it only for special purpose, like
	 * debugging. #OhmFactStoreView is way better, with the proper
	 * transaction support.
	 * <note>
	 * <para>
	 * This signal might be removed in the future.
	 * </para>
	 * </note>
	 **/
	g_signal_new ("removed", OHM_TYPE_FACT_STORE, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, OHM_TYPE_FACT);
	/**
	 * OhmFactStore::updated:
	 * @arg1: the updated fact.
	 *
	 * Emits ::updated when a #OhmFact is modified (a field is
	 * added/removed/updated) Use it only for special purpose,
	 * like debugging. #OhmFactStoreView is way better, with the
	 * proper transaction support.
	 * <note>
	 * <para>
	 * This signal might be removed in the future.
	 * </para>
	 * </note>
	 **/
	g_signal_new ("updated", OHM_TYPE_FACT_STORE, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_user_marshal_VOID__OBJECT_UINT_POINTER, G_TYPE_NONE, 3, OHM_TYPE_FACT, G_TYPE_UINT, G_TYPE_POINTER);
}


static void ohm_fact_store_init (OhmFactStore * self) {
	self->priv = OHM_FACT_STORE_GET_PRIVATE (self);

	self->priv->known_facts_qname = NULL;
	self->transaction = g_queue_new ();
}


static void ohm_fact_store_dispose (GObject * obj) {
	OhmFactStore * self;
	GSList* q_collection;
	GSList* q_it;

	self = OHM_FACT_STORE (obj);

	q_collection = self->priv->known_facts_qname;
	for (q_it = q_collection; q_it != NULL; q_it = q_it->next) {
		GQuark q;
		GSList* facts;

		q = GPOINTER_TO_INT (q_it->data);
		{
		  GSList* facts;
		  GSList* f_it;

		  facts = ohm_fact_store_get_facts_by_quark (self, q);
		  
		  for (f_it = facts; f_it != NULL; f_it = f_it->next) {
		    OhmFact* f;
		    f = (OhmFact*) f_it->data;
		    g_object_unref (G_OBJECT (f));
		  }
		}
		
		facts = g_object_get_qdata (G_OBJECT (self), q);
		g_slist_free (facts);
	}

	/* FIXME: interest.foreach ((DataForeachFunc)_delete_func);*/
	g_datalist_clear (&self->priv->interest);

	if (self->priv->known_facts_qname != NULL) {
	  g_slist_free (self->priv->known_facts_qname);
	  self->priv->known_facts_qname = NULL;
	}

	if (self->transaction != NULL) {
	  g_queue_free (self->transaction);
	  self->transaction = NULL;
	}

	G_OBJECT_CLASS (ohm_fact_store_parent_class)->dispose (obj);
}


GType ohm_fact_store_get_type (void) {
	static GType ohm_fact_store_type_id = 0;
	if (G_UNLIKELY (ohm_fact_store_type_id == 0)) {
	  static const GTypeInfo g_define_type_info = {
	    sizeof (OhmFactStoreClass),
	    (GBaseInitFunc) NULL,
	    (GBaseFinalizeFunc) NULL,
	    (GClassInitFunc) ohm_fact_store_class_init,
	    (GClassFinalizeFunc) NULL,
	    NULL,
	    sizeof (OhmFactStore),
	    0,
	    (GInstanceInitFunc) ohm_fact_store_init
	  };

	  ohm_fact_store_type_id = g_type_register_static (G_TYPE_OBJECT, "OhmFactStore", &g_define_type_info, 0);
	}

	return ohm_fact_store_type_id;
}

/**
 * ohm_get_fact_store:
 *
 * See #OhmFactStore for a description of the object returned.
 *
 * Returns: the singleton of the Ohm fact-store, not referenced (caller should not unref).
 **/
OhmFactStore* ohm_get_fact_store (void) {
	if (ohm_fs == NULL) {
	    ohm_fs = ohm_fact_store_new ();
	}
	return ohm_fs;
}


/**
 * ohm_value_from_string:
 * @str: a string
 *
 * Helper function, to wrap a string in a new #GValue.
 *
 * Returns: a new GValue which as the string value of @str.
 **/
GValue* ohm_value_from_string (const char* str) {
	GValue* value;

	g_return_val_if_fail (str != NULL, NULL);

	value = g_new0 (GValue, 1);
	g_value_init (value, G_TYPE_STRING);
	g_value_set_string (value, str);

	return value;
}


/**
 * ohm_value_from_int:
 * @val: an integer value
 *
 * Helper function, to wrap an int in a new #GValue.
 *
 * Returns: a new GValue which as the value of @val.
 **/
GValue* ohm_value_from_int (gint val) {
	GValue* value;

	value = g_new0 (GValue, 1);

	g_value_init (value, G_TYPE_INT);
	g_value_set_int (value, val);

	return value;
}


/**
 * ohm_value_from_structure:
 * @val: a #OhmStructure
 *
 * Helper function, to wrap a #OhmStructure in a new #GValue.
 *
 * Returns: a new GValue which as the value of @val.
 **/
GValue* ohm_value_from_structure (OhmStructure* val) {
	GValue* value;

	g_return_val_if_fail (OHM_IS_STRUCTURE (val), NULL);

	value = g_new0 (GValue, 1);

	g_value_init (value, OHM_TYPE_STRUCTURE);
	g_value_set_object (value, G_OBJECT (val));

	return value;
}


/**
 * ohm_value_from_fact:
 * @val: a #OhmFact
 *
 * Helper function, to wrap a #OhmFact in a new #GValue.
 *
 * Returns: a new GValue which as the value of @val.
 **/
GValue* ohm_value_from_fact (OhmFact* val) {
	GValue* value;

	g_return_val_if_fail (OHM_IS_FACT (val), NULL);

	value = g_new0 (GValue, 1);
	g_value_init (value, OHM_TYPE_FACT);
	g_value_set_object (value, G_OBJECT (val));

	return value;
}


/**
 * ohm_value_cmp:
 * @v1: a value
 * @v2: a value
 *
 * Helper function, to compare two #GValue.
 *
 * Returns: -diff, 0 or diff, if v1 is <, == or > than v2.
 **/
gint ohm_value_cmp (GValue* v1, GValue* v2) {
	if (v1 == v2) {
		return 0;
	}

	g_return_val_if_fail (G_VALUE_TYPE (v1) == G_VALUE_TYPE (v2), -1);

	if (G_VALUE_TYPE (v1) == G_TYPE_INT) {
		return g_value_get_int (v2) - g_value_get_int (v1);
	}

	if (G_VALUE_TYPE (v1) == G_TYPE_STRING) {
		return strcmp (g_value_get_string (v1), g_value_get_string (v2));
	}

	if (G_VALUE_TYPE (v1) == G_TYPE_BOOLEAN) {
		return g_value_get_boolean (v1) == g_value_get_boolean (v2);
	}

	if (G_VALUE_TYPE (v1) == G_TYPE_CHAR) {
		return g_value_get_char (v2) - g_value_get_char (v1);
	}

	/*   if (v1.type () == typeof (double))
	     return v2.get_double () - v1.get_double ();
	   if (v1.type () == Type.ENUM)
	     return (int) (v1.get_enum () == v2.get_enum ());
	   if (v1.type () == Type.FLAGS)
	     return (int) (v1.get_flags () == v2.get_flags ());*/

	if (G_VALUE_TYPE (v1) == G_TYPE_NONE) {
		return g_value_get_pointer (v1) == g_value_get_pointer (v2);
	}

	if (G_VALUE_TYPE (v1) == G_TYPE_OBJECT) {
		return g_value_get_object (v1) == g_value_get_object (v2);
	}

	if (G_VALUE_TYPE (v1) == G_TYPE_BOXED) {
		return g_value_get_boxed (v1) == g_value_get_boxed (v2);
	}

	return 0;
}


/**
 * ohm_value_get_structure:
 * @value: a GValue that owns a #OhmStructure
 *
 * Returns: a referenced value of the Structure hold by the GValue (or NULL in case of error).
 **/
OhmStructure* ohm_value_get_structure (const GValue* value) {
	g_return_val_if_fail (value != NULL, NULL);
	g_return_val_if_fail (G_VALUE_HOLDS (value, OHM_TYPE_STRUCTURE), NULL);

	return g_object_ref (OHM_STRUCTURE (g_value_get_object (value)));
}


/**
 * ohm_value_get_fact:
 * @value: a GValue that owns a #OhmFact
 *
 * Returns: a referenced value of the Fact hold by the GValue (or NULL in case of error).
 **/
OhmFact* ohm_value_get_fact (GValue* value) {
	g_return_val_if_fail (value != NULL, NULL);
	g_return_val_if_fail (G_VALUE_HOLDS (value, OHM_TYPE_FACT), NULL);

	return g_object_ref (OHM_FACT (g_value_get_object (value)));
}



static void g_cclosure_user_marshal_VOID__OBJECT_UINT_POINTER (GClosure * closure, GValue * return_value, guint n_param_values, const GValue * param_values, gpointer invocation_hint, gpointer marshal_data) {
	typedef void (*GMarshalFunc_VOID__OBJECT_UINT_POINTER) (gpointer data1, gpointer arg_1, GQuark arg_2, gpointer arg_3, gpointer data2);
	register GMarshalFunc_VOID__OBJECT_UINT_POINTER callback;
	register GCClosure * cc;
	register gpointer data1, data2;

	cc = ((GCClosure *) closure);
	g_return_if_fail (n_param_values == 4);
	if (G_CCLOSURE_SWAP_DATA (closure)) {
		data1 = closure->data;
		data2 = param_values->data[0].v_pointer;
	} else {
		data1 = param_values->data[0].v_pointer;
		data2 = closure->data;
	}
	callback = ((GMarshalFunc_VOID__OBJECT_UINT_POINTER) (marshal_data ? marshal_data : cc->callback));
	callback (data1, g_value_get_object (param_values + 1), g_value_get_uint (param_values + 2), g_value_get_pointer (param_values + 3), data2);
}




/* 
 * Local Variables:
 * c-basic-offset: 8
 * End:
 * vim:set shiftwidth=8:
 */

