
#include <prolog/ohm-fact.h>




struct _OhmStructurePrivate {
	GList* fields;
	GQuark _name;
};
#define OHM_STRUCTURE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), OHM_TYPE_STRUCTURE, OhmStructurePrivate))
enum  {
	OHM_STRUCTURE_DUMMY_PROPERTY,
	OHM_STRUCTURE_QNAME,
	OHM_STRUCTURE_NAME
};
static void ohm_structure_real_qset (OhmStructure* self, GQuark field, GValue* value);
static void _ohm_structure_value_to_string_gvalue_transform (GValue* src_value, GValue* dest_value);
static GObject * ohm_structure_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer ohm_structure_parent_class = NULL;
static void ohm_structure_dispose (GObject * obj);
struct _OhmPatternPrivate {
	OhmFactStoreView* _view;
};
#define OHM_PATTERN_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), OHM_TYPE_PATTERN, OhmPatternPrivate))
enum  {
	OHM_PATTERN_DUMMY_PROPERTY,
	OHM_PATTERN_VIEW
};
static void ohm_pattern_real_qset (OhmStructure* base, GQuark field, GValue* value);
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
	GList* facts;
	GData* interest;
};
#define OHM_FACT_STORE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), OHM_TYPE_FACT_STORE, OhmFactStorePrivate))
enum  {
	OHM_FACT_STORE_DUMMY_PROPERTY
};
static OhmFactStore* ohm_fact_store_fs;
static void _ohm_fact_store_update_views (OhmFactStore* self, OhmFact* fact, OhmFactStoreEvent event);
static void ohm_fact_store_set_view_interest (OhmFactStore* self, OhmFactStoreView* v, GSList* interested);
struct _OhmFactStoreChangeSetPrivate {
	GSList* _matches;
};
#define OHM_FACT_STORE_CHANGE_SET_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), OHM_FACT_STORE_TYPE_CHANGE_SET, OhmFactStoreChangeSetPrivate))
enum  {
	OHM_FACT_STORE_CHANGE_SET_DUMMY_PROPERTY,
	OHM_FACT_STORE_CHANGE_SET_MATCHES
};
static void ohm_fact_store_change_set_set_matches (OhmFactStoreChangeSet* self, GSList* value);
static gpointer ohm_fact_store_change_set_parent_class = NULL;
static void ohm_fact_store_change_set_dispose (GObject * obj);
struct _OhmFactStoreViewPrivate {
	GObject* _listener;
	OhmFactStore* _fact_store;
	GSList* _interested;
};
#define OHM_FACT_STORE_VIEW_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), OHM_FACT_STORE_TYPE_VIEW, OhmFactStoreViewPrivate))
enum  {
	OHM_FACT_STORE_VIEW_DUMMY_PROPERTY,
	OHM_FACT_STORE_VIEW_LISTENER,
	OHM_FACT_STORE_VIEW_FACT_STORE,
	OHM_FACT_STORE_VIEW_INTERESTED
};
static OhmFactStoreView* ohm_fact_store_view_new (OhmFactStore* fact_store, GObject* listener);
static void ohm_fact_store_view_set_listener (OhmFactStoreView* self, GObject* value);
static void ohm_fact_store_view_set_fact_store (OhmFactStoreView* self, OhmFactStore* value);
static GObject * ohm_fact_store_view_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer ohm_fact_store_view_parent_class = NULL;
static void ohm_fact_store_view_dispose (GObject * obj);
static gpointer ohm_fact_store_parent_class = NULL;
static void ohm_fact_store_dispose (GObject * obj);
struct _OhmRulePrivate {
	OhmFactStore* _fact_store;
	OhmFactStoreView* view;
	GSList* _provide;
};
#define OHM_RULE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), OHM_TYPE_RULE, OhmRulePrivate))
enum  {
	OHM_RULE_DUMMY_PROPERTY,
	OHM_RULE_FACT_STORE,
	OHM_RULE_INTERESTED,
	OHM_RULE_PROVIDE
};
static GObject * ohm_rule_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer ohm_rule_parent_class = NULL;
static void ohm_rule_dispose (GObject * obj);


static void g_cclosure_user_marshal_VOID__OBJECT_POINTER_POINTER (GClosure * closure, GValue * return_value, guint n_param_values, const GValue * param_values, gpointer invocation_hint, gpointer marshal_data);

/* foreach (int q in fields) {
   Quark qk = (Quark)q;
   Value *v = get_qdata (qk);
   free (v);
 }*/
OhmStructure* ohm_structure_new (const char* name) {
	GParameter * __params;
	GParameter * __params_it;
	OhmStructure * self;
	g_return_val_if_fail (name != NULL, NULL);
	__params = g_new0 (GParameter, 1);
	__params_it = __params;
	__params_it->name = "name";
	g_value_init (&__params_it->value, G_TYPE_STRING);
	g_value_set_string (&__params_it->value, name);
	__params_it++;
	self = g_object_newv (OHM_TYPE_STRUCTURE, __params_it - __params, __params);
	while (__params_it > __params) {
		--__params_it;
		g_value_unset (&__params_it->value);
	}
	g_free (__params);
	return self;
}


static void ohm_structure_real_qset (OhmStructure* self, GQuark field, GValue* value) {
	g_return_if_fail (OHM_IS_STRUCTURE (self));
	if (G_VALUE_TYPE (&(*value)) == G_TYPE_INVALID) {
		self->priv->fields = g_list_remove (self->priv->fields, field);
		g_object_set_qdata (G_OBJECT (self), field, NULL);
	} else {
		GValue* v;
		v = g_new0 (GValue, 1);
		g_value_init (v, G_VALUE_TYPE (&(*value)));
		*v = (*value);
		if (g_list_find (self->priv->fields, field) == NULL) {
			self->priv->fields = g_list_append (self->priv->fields, field);
		}
		g_object_set_qdata_full (G_OBJECT (self), field, ((gpointer*) v), g_free);
	}
}


void ohm_structure_qset (OhmStructure* self, GQuark field, GValue* value) {
	OHM_STRUCTURE_GET_CLASS (self)->qset (self, field, value);
}


GValue* ohm_structure_qget (OhmStructure* self, GQuark field) {
	g_return_val_if_fail (OHM_IS_STRUCTURE (self), NULL);
	return g_object_get_qdata (G_OBJECT (self), field);
}


void ohm_structure_set (OhmStructure* self, const char* field_name, GValue* value) {
	g_return_if_fail (OHM_IS_STRUCTURE (self));
	g_return_if_fail (field_name != NULL);
	ohm_structure_qset (self, g_quark_from_string (field_name), &(*value));
}


GValue* ohm_structure_get (OhmStructure* self, const char* field_name) {
	g_return_val_if_fail (OHM_IS_STRUCTURE (self), NULL);
	g_return_val_if_fail (field_name != NULL, NULL);
	return g_object_get_data (G_OBJECT (self), field_name);
}


char* ohm_structure_to_string (OhmStructure* self) {
	char* ret;
	gboolean first;
	char* _tmp4;
	g_return_val_if_fail (OHM_IS_STRUCTURE (self), NULL);
	ret = g_strdup_printf ("%s (", ohm_structure_get_name (self));
	first = TRUE;
	{
		GList* q_collection;
		GList* q_it;
		q_collection = self->priv->fields;
		for (q_it = q_collection; q_it != NULL; q_it = q_it->next) {
			gint q;
			q = GPOINTER_TO_INT (q_it->data);
			{
				GQuark qk;
				GValue* v;
				char* _tmp3;
				char* _tmp2;
				char* _tmp1;
				qk = ((GQuark) q);
				if (!first) {
					char* _tmp0;
					_tmp0 = NULL;
					ret = (_tmp0 = g_strconcat (ret, (", "), NULL), (ret = (g_free (ret), NULL)), _tmp0);
				} else {
					first = FALSE;
				}
				v = g_object_get_qdata (G_OBJECT (self), qk);
				_tmp3 = NULL;
				_tmp2 = NULL;
				_tmp1 = NULL;
				ret = (_tmp3 = g_strconcat (ret, (_tmp2 = (g_strdup_printf ("%s = %s", g_quark_to_string (qk), (_tmp1 = g_strdup_value_contents (v))))), NULL), (ret = (g_free (ret), NULL)), _tmp3);
				_tmp2 = (g_free (_tmp2), NULL);
				_tmp1 = (g_free (_tmp1), NULL);
			}
		}
	}
	_tmp4 = NULL;
	return (_tmp4 = g_strconcat (ret, ")", NULL), (ret = (g_free (ret), NULL)), _tmp4);
}


void ohm_structure_value_to_string (GValue* src, GValue* dest) {
	OhmStructure* s;
	char* _tmp0;
	s = ohm_value_get_structure (&(*src));
	g_return_if_fail (s != NULL);
	_tmp0 = NULL;
	g_value_set_string (&(*dest), (_tmp0 = ohm_structure_to_string (s)));
	_tmp0 = (g_free (_tmp0), NULL);
	(s == NULL ? NULL : (s = (g_object_unref (s), NULL)));
}


GQuark ohm_structure_get_qname (OhmStructure* self) {
	g_return_val_if_fail (OHM_IS_STRUCTURE (self), 0U);
	return self->priv->_name;
}


char* ohm_structure_get_name (OhmStructure* self) {
	g_return_val_if_fail (OHM_IS_STRUCTURE (self), NULL);
	return g_quark_to_string (self->priv->_name);
}


void ohm_structure_set_name (OhmStructure* self, const char* value) {
	g_return_if_fail (OHM_IS_STRUCTURE (self));
	self->priv->_name = g_quark_from_string (value);
}


static void _ohm_structure_value_to_string_gvalue_transform (GValue* src_value, GValue* dest_value) {
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
	{
		g_value_register_transform_func (OHM_TYPE_STRUCTURE, G_TYPE_STRING, _ohm_structure_value_to_string_gvalue_transform);
	}
	return obj;
}


static void ohm_structure_get_property (GObject * object, guint property_id, GValue * value, GParamSpec * pspec) {
	OhmStructure * self;
	self = OHM_STRUCTURE (object);
	switch (property_id) {
		case OHM_STRUCTURE_QNAME:
		g_value_set_pointer (value, ohm_structure_get_qname (self));
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
	g_type_class_add_private (klass, sizeof (OhmStructurePrivate));
	G_OBJECT_CLASS (klass)->get_property = ohm_structure_get_property;
	G_OBJECT_CLASS (klass)->set_property = ohm_structure_set_property;
	G_OBJECT_CLASS (klass)->constructor = ohm_structure_constructor;
	G_OBJECT_CLASS (klass)->dispose = ohm_structure_dispose;
	OHM_STRUCTURE_CLASS (klass)->qset = ohm_structure_real_qset;
	g_object_class_install_property (G_OBJECT_CLASS (klass), OHM_STRUCTURE_QNAME, g_param_spec_pointer ("qname", "qname", "qname", G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE));
	g_object_class_install_property (G_OBJECT_CLASS (klass), OHM_STRUCTURE_NAME, g_param_spec_string ("name", "name", "name", NULL, G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));
}


static void ohm_structure_init (OhmStructure * self) {
	self->priv = OHM_STRUCTURE_GET_PRIVATE (self);
	self->priv->fields = NULL;
}


static void ohm_structure_dispose (GObject * obj) {
	OhmStructure * self;
	self = OHM_STRUCTURE (obj);
	{
	}
	(self->priv->fields == NULL ? NULL : (self->priv->fields = (g_list_free (self->priv->fields), NULL)));
	G_OBJECT_CLASS (ohm_structure_parent_class)->dispose (obj);
}


GType ohm_structure_get_type (void) {
	static GType ohm_structure_type_id = 0;
	if (G_UNLIKELY (ohm_structure_type_id == 0)) {
		static const GTypeInfo g_define_type_info = { sizeof (OhmStructureClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) ohm_structure_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (OhmStructure), 0, (GInstanceInitFunc) ohm_structure_init };
		ohm_structure_type_id = g_type_register_static (G_TYPE_OBJECT, "OhmStructure", &g_define_type_info, 0);
	}
	return ohm_structure_type_id;
}


OhmPattern* ohm_pattern_new (const char* name) {
	GParameter * __params;
	GParameter * __params_it;
	OhmPattern * self;
	g_return_val_if_fail (name != NULL, NULL);
	__params = g_new0 (GParameter, 1);
	__params_it = __params;
	__params_it->name = "name";
	g_value_init (&__params_it->value, G_TYPE_STRING);
	g_value_set_string (&__params_it->value, name);
	__params_it++;
	self = g_object_newv (OHM_TYPE_PATTERN, __params_it - __params, __params);
	while (__params_it > __params) {
		--__params_it;
		g_value_unset (&__params_it->value);
	}
	g_free (__params);
	return self;
}


static void ohm_pattern_real_qset (OhmStructure* base, GQuark field, GValue* value) {
	OhmPattern * self;
	self = OHM_PATTERN (base);
	g_debug ("ohm-fact.vala:107: field matching not implemented");
}


OhmPatternMatch* ohm_pattern_match (OhmPattern* self, OhmFact* fact, OhmFactStoreEvent event) {
	g_return_val_if_fail (OHM_IS_PATTERN (self), NULL);
	g_return_val_if_fail (OHM_IS_FACT (fact), NULL);
	if (ohm_structure_get_qname (OHM_STRUCTURE (fact)) != ohm_structure_get_qname (OHM_STRUCTURE (self))) {
		return NULL;
	}
	/*           var matches = new HashTable<Variable,Value>(direct_hash, direct_equal);
	           foreach (string field in pattern.map.get_keys ()) {
	             Value* fact_val = fact.get (field);
	             Value* patt_val = pattern.get (field);
	             if (fact_val == null)
	               return null;
	             if (patt_val->holds (typeof (Variable))) {
	               matches.insert (value_get_variable (*patt_val), fact_val);
	             } else if (patt_val->type () != fact_val->type ()) {
	               return null;
	               // FIXME: use value compare func (proposal to do here)
	             } else if (patt_val->strdup_contents () != fact_val->strdup_contents ()) {
	               return null;
	             }
	           }*/
	return ohm_pattern_match_new (fact, self, event);
}


OhmFactStoreView* ohm_pattern_get_view (OhmPattern* self) {
	g_return_val_if_fail (OHM_IS_PATTERN (self), NULL);
	return self->priv->_view;
}


void ohm_pattern_set_view (OhmPattern* self, OhmFactStoreView* value) {
	OhmFactStoreView* _tmp2;
	OhmFactStoreView* _tmp1;
	g_return_if_fail (OHM_IS_PATTERN (self));
	_tmp2 = NULL;
	_tmp1 = NULL;
	self->priv->_view = (_tmp2 = (_tmp1 = value, (_tmp1 == NULL ? NULL : g_object_ref (_tmp1))), (self->priv->_view == NULL ? NULL : (self->priv->_view = (g_object_unref (self->priv->_view), NULL))), _tmp2);
}


/*         private void _to_string (pointer key, pointer value, pointer user_data) {
           Variable* v = key;
           Value* val = value;
           string* ret = user_data;
           string s = *ret + "(%s, %s)".printf (v->name, val->strdup_contents ());
           ret = &s;
         }*/
OhmPatternMatch* ohm_pattern_match_new (OhmFact* fact, OhmPattern* pattern, OhmFactStoreEvent event) {
	GParameter * __params;
	GParameter * __params_it;
	OhmPatternMatch * self;
	g_return_val_if_fail (OHM_IS_FACT (fact), NULL);
	g_return_val_if_fail (OHM_IS_PATTERN (pattern), NULL);
	__params = g_new0 (GParameter, 3);
	__params_it = __params;
	__params_it->name = "fact";
	g_value_init (&__params_it->value, OHM_TYPE_FACT);
	g_value_set_object (&__params_it->value, fact);
	__params_it++;
	__params_it->name = "pattern";
	g_value_init (&__params_it->value, OHM_TYPE_PATTERN);
	g_value_set_object (&__params_it->value, pattern);
	__params_it++;
	__params_it->name = "event";
	g_value_init (&__params_it->value, OHM_FACT_STORE_TYPE_EVENT);
	g_value_set_enum (&__params_it->value, event);
	__params_it++;
	self = g_object_newv (OHM_PATTERN_TYPE_MATCH, __params_it - __params, __params);
	while (__params_it > __params) {
		--__params_it;
		g_value_unset (&__params_it->value);
	}
	g_free (__params);
	return self;
}


char* ohm_pattern_match_to_string (OhmPatternMatch* self) {
	char* _tmp1;
	char* _tmp0;
	char* _tmp2;
	g_return_val_if_fail (OHM_PATTERN_IS_MATCH (self), NULL);
	_tmp1 = NULL;
	_tmp0 = NULL;
	_tmp2 = NULL;
	return (_tmp2 = g_strdup_printf ("a match: %s, %s", (_tmp0 = ohm_structure_to_string (OHM_STRUCTURE (self->priv->_fact))), (_tmp1 = ohm_structure_to_string (OHM_STRUCTURE (self->priv->_pattern)))), (_tmp1 = (g_free (_tmp1), NULL)), (_tmp0 = (g_free (_tmp0), NULL)), _tmp2);
}


OhmFact* ohm_pattern_match_get_fact (OhmPatternMatch* self) {
	g_return_val_if_fail (OHM_PATTERN_IS_MATCH (self), NULL);
	return self->priv->_fact;
}


static void ohm_pattern_match_set_fact (OhmPatternMatch* self, OhmFact* value) {
	OhmFact* _tmp2;
	OhmFact* _tmp1;
	g_return_if_fail (OHM_PATTERN_IS_MATCH (self));
	_tmp2 = NULL;
	_tmp1 = NULL;
	self->priv->_fact = (_tmp2 = (_tmp1 = value, (_tmp1 == NULL ? NULL : g_object_ref (_tmp1))), (self->priv->_fact == NULL ? NULL : (self->priv->_fact = (g_object_unref (self->priv->_fact), NULL))), _tmp2);
}


OhmPattern* ohm_pattern_match_get_pattern (OhmPatternMatch* self) {
	g_return_val_if_fail (OHM_PATTERN_IS_MATCH (self), NULL);
	return self->priv->_pattern;
}


static void ohm_pattern_match_set_pattern (OhmPatternMatch* self, OhmPattern* value) {
	OhmPattern* _tmp2;
	OhmPattern* _tmp1;
	g_return_if_fail (OHM_PATTERN_IS_MATCH (self));
	_tmp2 = NULL;
	_tmp1 = NULL;
	self->priv->_pattern = (_tmp2 = (_tmp1 = value, (_tmp1 == NULL ? NULL : g_object_ref (_tmp1))), (self->priv->_pattern == NULL ? NULL : (self->priv->_pattern = (g_object_unref (self->priv->_pattern), NULL))), _tmp2);
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
	g_object_class_install_property (G_OBJECT_CLASS (klass), OHM_PATTERN_MATCH_FACT, g_param_spec_object ("fact", "fact", "fact", OHM_TYPE_FACT, G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (G_OBJECT_CLASS (klass), OHM_PATTERN_MATCH_PATTERN, g_param_spec_object ("pattern", "pattern", "pattern", OHM_TYPE_PATTERN, G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (G_OBJECT_CLASS (klass), OHM_PATTERN_MATCH_EVENT, g_param_spec_enum ("event", "event", "event", OHM_FACT_STORE_TYPE_EVENT, 0, G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
}


static void ohm_pattern_match_init (OhmPatternMatch * self) {
	self->priv = OHM_PATTERN_MATCH_GET_PRIVATE (self);
}


static void ohm_pattern_match_dispose (GObject * obj) {
	OhmPatternMatch * self;
	self = OHM_PATTERN_MATCH (obj);
	(self->priv->_fact == NULL ? NULL : (self->priv->_fact = (g_object_unref (self->priv->_fact), NULL)));
	(self->priv->_pattern == NULL ? NULL : (self->priv->_pattern = (g_object_unref (self->priv->_pattern), NULL)));
	G_OBJECT_CLASS (ohm_pattern_match_parent_class)->dispose (obj);
}


GType ohm_pattern_match_get_type (void) {
	static GType ohm_pattern_match_type_id = 0;
	if (G_UNLIKELY (ohm_pattern_match_type_id == 0)) {
		static const GTypeInfo g_define_type_info = { sizeof (OhmPatternMatchClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) ohm_pattern_match_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (OhmPatternMatch), 0, (GInstanceInitFunc) ohm_pattern_match_init };
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
	OHM_STRUCTURE_CLASS (klass)->qset = ohm_pattern_real_qset;
	g_object_class_install_property (G_OBJECT_CLASS (klass), OHM_PATTERN_VIEW, g_param_spec_object ("view", "view", "view", OHM_FACT_STORE_TYPE_VIEW, G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE));
}


static void ohm_pattern_init (OhmPattern * self) {
	self->priv = OHM_PATTERN_GET_PRIVATE (self);
	self->priv->_view = NULL;
}


static void ohm_pattern_dispose (GObject * obj) {
	OhmPattern * self;
	self = OHM_PATTERN (obj);
	(self->priv->_view == NULL ? NULL : (self->priv->_view = (g_object_unref (self->priv->_view), NULL)));
	G_OBJECT_CLASS (ohm_pattern_parent_class)->dispose (obj);
}


GType ohm_pattern_get_type (void) {
	static GType ohm_pattern_type_id = 0;
	if (G_UNLIKELY (ohm_pattern_type_id == 0)) {
		static const GTypeInfo g_define_type_info = { sizeof (OhmPatternClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) ohm_pattern_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (OhmPattern), 0, (GInstanceInitFunc) ohm_pattern_init };
		ohm_pattern_type_id = g_type_register_static (OHM_TYPE_STRUCTURE, "OhmPattern", &g_define_type_info, 0);
	}
	return ohm_pattern_type_id;
}


OhmFact* ohm_fact_new (const char* name) {
	GParameter * __params;
	GParameter * __params_it;
	OhmFact * self;
	g_return_val_if_fail (name != NULL, NULL);
	__params = g_new0 (GParameter, 1);
	__params_it = __params;
	__params_it->name = "name";
	g_value_init (&__params_it->value, G_TYPE_STRING);
	g_value_set_string (&__params_it->value, name);
	__params_it++;
	self = g_object_newv (OHM_TYPE_FACT, __params_it - __params, __params);
	while (__params_it > __params) {
		--__params_it;
		g_value_unset (&__params_it->value);
	}
	g_free (__params);
	return self;
}


/*fact_store.remove (this);*/
GValue* ohm_fact_get (OhmFact* self, const char* field_name) {
	g_return_val_if_fail (OHM_IS_FACT (self), NULL);
	g_return_val_if_fail (field_name != NULL, NULL);
	return ohm_structure_get (OHM_STRUCTURE (self), field_name);
}


void ohm_fact_set (OhmFact* self, const char* field_name, GValue* value) {
	g_return_if_fail (OHM_IS_FACT (self));
	g_return_if_fail (field_name != NULL);
	ohm_structure_set (OHM_STRUCTURE (self), field_name, &(*value));
}


/* rationale of virtual method: signal should come only from FactStore/ChangeSet*/
static void ohm_fact_real_qset (OhmStructure* base, GQuark field, GValue* value) {
	OhmFact * self;
	self = OHM_FACT (base);
	OHM_STRUCTURE_CLASS (ohm_fact_parent_class)->qset (OHM_STRUCTURE (self), field, &(*value));
	if (self->priv->_fact_store != NULL) {
		ohm_fact_store_update (self->priv->_fact_store, self, field, &(*value));
	}
}


OhmFactStore* ohm_fact_get_fact_store (OhmFact* self) {
	g_return_val_if_fail (OHM_IS_FACT (self), NULL);
	return self->priv->_fact_store;
}

GList *ohm_fact_get_fields(OhmFact *self) {
	g_return_val_if_fail (OHM_IS_FACT (self), NULL);
	return OHM_STRUCTURE(self)->priv->fields;
}


void ohm_fact_set_fact_store (OhmFact* self, OhmFactStore* value) {
	OhmFactStore* _tmp2;
	OhmFactStore* _tmp1;
	g_return_if_fail (OHM_IS_FACT (self));
	_tmp2 = NULL;
	_tmp1 = NULL;
	self->priv->_fact_store = (_tmp2 = (_tmp1 = value, (_tmp1 == NULL ? NULL : g_object_ref (_tmp1))), (self->priv->_fact_store == NULL ? NULL : (self->priv->_fact_store = (g_object_unref (self->priv->_fact_store), NULL))), _tmp2);
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
	g_object_class_install_property (G_OBJECT_CLASS (klass), OHM_FACT_FACT_STORE, g_param_spec_object ("fact-store", "fact-store", "fact-store", OHM_TYPE_FACT_STORE, G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE));
}


static void ohm_fact_init (OhmFact * self) {
	self->priv = OHM_FACT_GET_PRIVATE (self);
	self->priv->_fact_store = NULL;
}


static void ohm_fact_dispose (GObject * obj) {
	OhmFact * self;
	self = OHM_FACT (obj);
	{
		if (self->priv->_fact_store != NULL) {
		}
	}
	(self->priv->_fact_store == NULL ? NULL : (self->priv->_fact_store = (g_object_unref (self->priv->_fact_store), NULL)));
	G_OBJECT_CLASS (ohm_fact_parent_class)->dispose (obj);
}


GType ohm_fact_get_type (void) {
	static GType ohm_fact_type_id = 0;
	if (G_UNLIKELY (ohm_fact_type_id == 0)) {
		static const GTypeInfo g_define_type_info = { sizeof (OhmFactClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) ohm_fact_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (OhmFact), 0, (GInstanceInitFunc) ohm_fact_init };
		ohm_fact_type_id = g_type_register_static (OHM_TYPE_STRUCTURE, "OhmFact", &g_define_type_info, 0);
	}
	return ohm_fact_type_id;
}


static void _ohm_fact_store_update_views (OhmFactStore* self, OhmFact* fact, OhmFactStoreEvent event) {
	GData* _tmp0 = {0};
	GSList* patterns;
	g_return_if_fail (OHM_IS_FACT_STORE (self));
	g_return_if_fail (OHM_IS_FACT (fact));
	patterns = ((GSList*) g_datalist_id_get_data ((_tmp0 = self->priv->interest, &_tmp0), ohm_structure_get_qname (OHM_STRUCTURE (fact))));
	g_message ("ohm-fact.vala:213: patterns %d", g_slist_length (patterns));
	{
		GSList* p_collection;
		GSList* p_it;
		p_collection = patterns;
		for (p_it = p_collection; p_it != NULL; p_it = p_it->next) {
			OhmPattern* _tmp1;
			OhmPattern* p;
			_tmp1 = NULL;
			p = (_tmp1 = ((OhmPattern*) p_it->data), (_tmp1 == NULL ? NULL : g_object_ref (_tmp1)));
			{
				OhmPatternMatch* m;
				m = ohm_pattern_match (p, fact, event);
				ohm_fact_store_change_set_add_match (ohm_pattern_get_view (p)->change_set, m);
				(p == NULL ? NULL : (p = (g_object_unref (p), NULL)));
				(m == NULL ? NULL : (m = (g_object_unref (m), NULL)));
			}
		}
	}
}


gboolean ohm_fact_store_insert (OhmFactStore* self, OhmFact* fact) {
	GSList* facts;
	g_return_val_if_fail (OHM_IS_FACT_STORE (self), FALSE);
	g_return_val_if_fail (OHM_IS_FACT (fact), FALSE);
	facts = ohm_fact_store_get_facts_by_quark (self, ohm_structure_get_qname (OHM_STRUCTURE (fact)));
	if (ohm_fact_get_fact_store (fact) != NULL) {
		char* _tmp0;
		_tmp0 = NULL;
		g_warning ("ohm-fact.vala:227: Fact %s has already a factStore", (_tmp0 = ohm_structure_to_string (OHM_STRUCTURE (fact))));
		_tmp0 = (g_free (_tmp0), NULL);
		return FALSE;
	}
	if (g_slist_find (facts, fact) == NULL) {
		OhmFact* _tmp2;
		ohm_fact_set_fact_store (fact, self);
		/* FIXME check unref facts!*/
		_tmp2 = NULL;
		facts = g_slist_prepend (facts, (_tmp2 = fact, (_tmp2 == NULL ? NULL : g_object_ref (_tmp2))));
		g_object_set_qdata (G_OBJECT (self), ohm_structure_get_qname (OHM_STRUCTURE (fact)), ((gpointer*) facts));
		if (g_list_find (self->priv->facts, ohm_structure_get_qname (OHM_STRUCTURE (fact))) == NULL) {
			self->priv->facts = g_list_prepend (self->priv->facts, ohm_structure_get_qname (OHM_STRUCTURE (fact)));
		}
		g_signal_emit_by_name (G_OBJECT (self), "inserted", fact);
#if 0
		_ohm_fact_store_update_views (self, fact, OHM_FACT_STORE_EVENT_ADDED);
#endif
		return TRUE;
	}
	return FALSE;
}


void ohm_fact_store_remove (OhmFactStore* self, OhmFact* fact) {
	GSList* facts;
	GSList* found;
	g_return_if_fail (OHM_IS_FACT_STORE (self));
	g_return_if_fail (OHM_IS_FACT (fact));
	facts = ohm_fact_store_get_facts_by_quark (self, ohm_structure_get_qname (OHM_STRUCTURE (fact)));
	found = g_slist_find (facts, fact);
	if (found != NULL) {
		facts = g_slist_delete_link (facts, found);
		g_object_set_qdata (G_OBJECT (self), ohm_structure_get_qname (OHM_STRUCTURE (fact)), ((gpointer*) facts));
#if 0
		_ohm_fact_store_update_views (self, fact, OHM_FACT_STORE_EVENT_REMOVED);
#endif

		ohm_fact_set_fact_store (fact, NULL);
		g_signal_emit_by_name (G_OBJECT (self), "removed", fact);
	}
}


void ohm_fact_store_update (OhmFactStore* self, OhmFact* fact, GQuark field, GValue* value) {
	g_return_if_fail (OHM_IS_FACT_STORE (self));
	g_return_if_fail (OHM_IS_FACT (fact));
	g_signal_emit_by_name (G_OBJECT (self), "updated", fact, field, &(*value));
	_ohm_fact_store_update_views (self, fact, OHM_FACT_STORE_EVENT_UPDATED);
}


GSList* ohm_fact_store_get_facts_by_quark (OhmFactStore* self, GQuark qname) {
	g_return_val_if_fail (OHM_IS_FACT_STORE (self), NULL);
	return ((GSList*) g_object_get_qdata (G_OBJECT (self), qname));
}


GSList* ohm_fact_store_get_facts_by_name (OhmFactStore* self, const char* name) {
	g_return_val_if_fail (OHM_IS_FACT_STORE (self), NULL);
	g_return_val_if_fail (name != NULL, NULL);
	return ((GSList*) g_object_get_data (G_OBJECT (self), name));
}


GSList* ohm_fact_store_get_facts_by_pattern (OhmFactStore* self, OhmPattern* pattern) {
	g_return_val_if_fail (OHM_IS_FACT_STORE (self), NULL);
	g_return_val_if_fail (OHM_IS_PATTERN (pattern), NULL);
	return ((GSList*) g_object_get_qdata (G_OBJECT (self), ohm_structure_get_qname (OHM_STRUCTURE (pattern))));
}


OhmFactStore* ohm_fact_store_new (void) {
	OhmFactStore * self;
	self = g_object_newv (OHM_TYPE_FACT_STORE, 0, NULL);
	return self;
}


char* ohm_fact_store_to_string (OhmFactStore* self) {
	char* ret;
	g_return_val_if_fail (OHM_IS_FACT_STORE (self), NULL);
	ret = g_strdup_printf ("FactStore %p:\n\n", self);
	{
		GList* q_collection;
		GList* q_it;
		q_collection = self->priv->facts;
		for (q_it = q_collection; q_it != NULL; q_it = q_it->next) {
			gint q;
			q = GPOINTER_TO_INT (q_it->data);
			{
				{
					GSList* f_collection;
					GSList* f_it;
					f_collection = ohm_fact_store_get_facts_by_quark (self, ((GQuark) q));
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
	}
	return ret;
}


void ohm_fact_store_my_debug (OhmFactStore* self) {
	char* _tmp0;
	g_return_if_fail (OHM_IS_FACT_STORE (self));
	_tmp0 = NULL;
	g_debug ((_tmp0 = ohm_fact_store_to_string (self)));
	_tmp0 = (g_free (_tmp0), NULL);
}


OhmFactStore* ohm_fact_store_get_fact_store (void) {
	OhmFactStore* _tmp1;
	if (ohm_fact_store_fs == NULL) {
		OhmFactStore* _tmp0;
		_tmp0 = NULL;
		ohm_fact_store_fs = (_tmp0 = ohm_fact_store_new (), (ohm_fact_store_fs == NULL ? NULL : (ohm_fact_store_fs = (g_object_unref (ohm_fact_store_fs), NULL))), _tmp0);
	}
	_tmp1 = NULL;
	return (_tmp1 = ohm_fact_store_fs, (_tmp1 == NULL ? NULL : g_object_ref (_tmp1)));
}


OhmFactStoreView* ohm_fact_store_new_view (OhmFactStore* self, GObject* listener) {
	g_return_val_if_fail (OHM_IS_FACT_STORE (self), NULL);
	g_return_val_if_fail (listener == NULL || G_IS_OBJECT (listener), NULL);
	return ohm_fact_store_view_new (self, listener);
}


static void ohm_fact_store_set_view_interest (OhmFactStore* self, OhmFactStoreView* v, GSList* interested) {
	g_return_if_fail (OHM_IS_FACT_STORE (self));
	g_return_if_fail (OHM_FACT_STORE_IS_VIEW (v));
	g_return_if_fail (interested != NULL);
	/* why here?*/
	{
		GSList* p_collection;
		GSList* p_it;
		p_collection = interested;
		for (p_it = p_collection; p_it != NULL; p_it = p_it->next) {
			OhmPattern* _tmp3;
			OhmPattern* p;
			_tmp3 = NULL;
			p = (_tmp3 = ((OhmPattern*) p_it->data), (_tmp3 == NULL ? NULL : g_object_ref (_tmp3)));
			{
				GData* _tmp0 = {0};
				GSList* patterns;
#if 0
				patterns = ((GSList*) g_datalist_id_get_data ((_tmp0 = self->priv->interest, &_tmp0), ohm_structure_get_qname (OHM_STRUCTURE (p))));
#else
				patterns = ((GSList*) g_datalist_id_get_data (&self->priv->interest, ohm_structure_get_qname (OHM_STRUCTURE (p))));
#endif
				if (g_slist_find (patterns, p) == NULL) {
					OhmPattern* _tmp1;
					GData* _tmp2 = {0};
					_tmp1 = NULL;
					patterns = g_slist_prepend (patterns, (_tmp1 = p, (_tmp1 == NULL ? NULL : g_object_ref (_tmp1))));
					/* FIXME: match now?*/
					ohm_pattern_set_view (p, v);
#if 0
					g_datalist_id_set_data_full ((_tmp2 = self->priv->interest, &_tmp2), ohm_structure_get_qname (OHM_STRUCTURE (p)), patterns, NULL);
#else
					g_datalist_id_set_data_full (&self->priv->interest, ohm_structure_get_qname (OHM_STRUCTURE (p)), patterns, NULL);
#endif
				}
				(p == NULL ? NULL : (p = (g_object_unref (p), NULL)));
			}
		}
	}
}


void ohm_fact_store_change_set_add_match (OhmFactStoreChangeSet* self, OhmPatternMatch* match) {
	OhmPatternMatch* _tmp0;
	g_return_if_fail (OHM_FACT_STORE_IS_CHANGE_SET (self));
	g_return_if_fail (OHM_PATTERN_IS_MATCH (match));
	_tmp0 = NULL;
	self->priv->_matches = g_slist_prepend (self->priv->_matches, (_tmp0 = match, (_tmp0 == NULL ? NULL : g_object_ref (_tmp0))));
}


void ohm_fact_store_change_set_reset (OhmFactStoreChangeSet* self) {
	g_return_if_fail (OHM_FACT_STORE_IS_CHANGE_SET (self));
	ohm_fact_store_change_set_set_matches (self, NULL);
}


OhmFactStoreChangeSet* ohm_fact_store_change_set_new (void) {
	OhmFactStoreChangeSet * self;
	self = g_object_newv (OHM_FACT_STORE_TYPE_CHANGE_SET, 0, NULL);
	return self;
}


GSList* ohm_fact_store_change_set_get_matches (OhmFactStoreChangeSet* self) {
	g_return_val_if_fail (OHM_FACT_STORE_IS_CHANGE_SET (self), NULL);
	return self->priv->_matches;
}


static void ohm_fact_store_change_set_set_matches (OhmFactStoreChangeSet* self, GSList* value) {
	g_return_if_fail (OHM_FACT_STORE_IS_CHANGE_SET (self));
	self->priv->_matches = value;
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


static void ohm_fact_store_change_set_set_property (GObject * object, guint property_id, const GValue * value, GParamSpec * pspec) {
	OhmFactStoreChangeSet * self;
	self = OHM_FACT_STORE_CHANGE_SET (object);
	switch (property_id) {
		case OHM_FACT_STORE_CHANGE_SET_MATCHES:
		ohm_fact_store_change_set_set_matches (self, g_value_get_pointer (value));
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
	G_OBJECT_CLASS (klass)->set_property = ohm_fact_store_change_set_set_property;
	G_OBJECT_CLASS (klass)->dispose = ohm_fact_store_change_set_dispose;
	g_object_class_install_property (G_OBJECT_CLASS (klass), OHM_FACT_STORE_CHANGE_SET_MATCHES, g_param_spec_pointer ("matches", "matches", "matches", G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE));
}


static void ohm_fact_store_change_set_init (OhmFactStoreChangeSet * self) {
	self->priv = OHM_FACT_STORE_CHANGE_SET_GET_PRIVATE (self);
}


static void ohm_fact_store_change_set_dispose (GObject * obj) {
	OhmFactStoreChangeSet * self;
	self = OHM_FACT_STORE_CHANGE_SET (obj);
	G_OBJECT_CLASS (ohm_fact_store_change_set_parent_class)->dispose (obj);
}


GType ohm_fact_store_change_set_get_type (void) {
	static GType ohm_fact_store_change_set_type_id = 0;
	if (G_UNLIKELY (ohm_fact_store_change_set_type_id == 0)) {
		static const GTypeInfo g_define_type_info = { sizeof (OhmFactStoreChangeSetClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) ohm_fact_store_change_set_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (OhmFactStoreChangeSet), 0, (GInstanceInitFunc) ohm_fact_store_change_set_init };
		ohm_fact_store_change_set_type_id = g_type_register_static (G_TYPE_OBJECT, "OhmFactStoreChangeSet", &g_define_type_info, 0);
	}
	return ohm_fact_store_change_set_type_id;
}


static OhmFactStoreView* ohm_fact_store_view_new (OhmFactStore* fact_store, GObject* listener) {
	GParameter * __params;
	GParameter * __params_it;
	OhmFactStoreView * self;
	g_return_val_if_fail (OHM_IS_FACT_STORE (fact_store), NULL);
	g_return_val_if_fail (listener == NULL || G_IS_OBJECT (listener), NULL);
	__params = g_new0 (GParameter, 2);
	__params_it = __params;
	__params_it->name = "fact-store";
	g_value_init (&__params_it->value, OHM_TYPE_FACT_STORE);
	g_value_set_object (&__params_it->value, fact_store);
	__params_it++;
	__params_it->name = "listener";
	g_value_init (&__params_it->value, G_TYPE_OBJECT);
	g_value_set_object (&__params_it->value, listener);
	__params_it++;
	self = g_object_newv (OHM_FACT_STORE_TYPE_VIEW, __params_it - __params, __params);
	while (__params_it > __params) {
		--__params_it;
		g_value_unset (&__params_it->value);
	}
	g_free (__params);
	return self;
}


GObject* ohm_fact_store_view_get_listener (OhmFactStoreView* self) {
	g_return_val_if_fail (OHM_FACT_STORE_IS_VIEW (self), NULL);
	return self->priv->_listener;
}


static void ohm_fact_store_view_set_listener (OhmFactStoreView* self, GObject* value) {
	GObject* _tmp2;
	GObject* _tmp1;
	g_return_if_fail (OHM_FACT_STORE_IS_VIEW (self));
	_tmp2 = NULL;
	_tmp1 = NULL;
	self->priv->_listener = (_tmp2 = (_tmp1 = value, (_tmp1 == NULL ? NULL : g_object_ref (_tmp1))), (self->priv->_listener == NULL ? NULL : (self->priv->_listener = (g_object_unref (self->priv->_listener), NULL))), _tmp2);
}


OhmFactStore* ohm_fact_store_view_get_fact_store (OhmFactStoreView* self) {
	g_return_val_if_fail (OHM_FACT_STORE_IS_VIEW (self), NULL);
	return self->priv->_fact_store;
}


static void ohm_fact_store_view_set_fact_store (OhmFactStoreView* self, OhmFactStore* value) {
	OhmFactStore* _tmp2;
	OhmFactStore* _tmp1;
	g_return_if_fail (OHM_FACT_STORE_IS_VIEW (self));
	_tmp2 = NULL;
	_tmp1 = NULL;
	self->priv->_fact_store = (_tmp2 = (_tmp1 = value, (_tmp1 == NULL ? NULL : g_object_ref (_tmp1))), (self->priv->_fact_store == NULL ? NULL : (self->priv->_fact_store = (g_object_unref (self->priv->_fact_store), NULL))), _tmp2);
}


GSList* ohm_fact_store_view_get_interested (OhmFactStoreView* self) {
	g_return_val_if_fail (OHM_FACT_STORE_IS_VIEW (self), NULL);
	return self->priv->_interested;
}


void ohm_fact_store_view_set_interested (OhmFactStoreView* self, GSList* value) {
	g_return_if_fail (OHM_FACT_STORE_IS_VIEW (self));
	self->priv->_interested = value;
	ohm_fact_store_set_view_interest (self->priv->_fact_store, self, self->priv->_interested);
}


static GObject * ohm_fact_store_view_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	OhmFactStoreViewClass * klass;
	GObjectClass * parent_class;
	OhmFactStoreView * self;
	klass = OHM_FACT_STORE_VIEW_CLASS (g_type_class_peek (OHM_FACT_STORE_TYPE_VIEW));
	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);
	self = OHM_FACT_STORE_VIEW (obj);
	{
		OhmFactStoreChangeSet* _tmp0;
		_tmp0 = NULL;
		self->change_set = (_tmp0 = ohm_fact_store_change_set_new (), (self->change_set == NULL ? NULL : (self->change_set = (g_object_unref (self->change_set), NULL))), _tmp0);
	}
	return obj;
}


static void ohm_fact_store_view_get_property (GObject * object, guint property_id, GValue * value, GParamSpec * pspec) {
	OhmFactStoreView * self;
	self = OHM_FACT_STORE_VIEW (object);
	switch (property_id) {
		case OHM_FACT_STORE_VIEW_LISTENER:
		g_value_set_object (value, ohm_fact_store_view_get_listener (self));
		break;
		case OHM_FACT_STORE_VIEW_FACT_STORE:
		g_value_set_object (value, ohm_fact_store_view_get_fact_store (self));
		break;
		case OHM_FACT_STORE_VIEW_INTERESTED:
		g_value_set_pointer (value, ohm_fact_store_view_get_interested (self));
		break;
		default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}


static void ohm_fact_store_view_set_property (GObject * object, guint property_id, const GValue * value, GParamSpec * pspec) {
	OhmFactStoreView * self;
	self = OHM_FACT_STORE_VIEW (object);
	switch (property_id) {
		case OHM_FACT_STORE_VIEW_LISTENER:
		ohm_fact_store_view_set_listener (self, g_value_get_object (value));
		break;
		case OHM_FACT_STORE_VIEW_FACT_STORE:
		ohm_fact_store_view_set_fact_store (self, g_value_get_object (value));
		break;
		case OHM_FACT_STORE_VIEW_INTERESTED:
		ohm_fact_store_view_set_interested (self, g_value_get_pointer (value));
		break;
		default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}


static void ohm_fact_store_view_class_init (OhmFactStoreViewClass * klass) {
	ohm_fact_store_view_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (OhmFactStoreViewPrivate));
	G_OBJECT_CLASS (klass)->get_property = ohm_fact_store_view_get_property;
	G_OBJECT_CLASS (klass)->set_property = ohm_fact_store_view_set_property;
	G_OBJECT_CLASS (klass)->constructor = ohm_fact_store_view_constructor;
	G_OBJECT_CLASS (klass)->dispose = ohm_fact_store_view_dispose;
	g_object_class_install_property (G_OBJECT_CLASS (klass), OHM_FACT_STORE_VIEW_LISTENER, g_param_spec_object ("listener", "listener", "listener", G_TYPE_OBJECT, G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));
	g_object_class_install_property (G_OBJECT_CLASS (klass), OHM_FACT_STORE_VIEW_FACT_STORE, g_param_spec_object ("fact-store", "fact-store", "fact-store", OHM_TYPE_FACT_STORE, G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));
	g_object_class_install_property (G_OBJECT_CLASS (klass), OHM_FACT_STORE_VIEW_INTERESTED, g_param_spec_pointer ("interested", "interested", "interested", G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE));
	g_signal_new ("updated", OHM_FACT_STORE_TYPE_VIEW, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, OHM_FACT_STORE_TYPE_CHANGE_SET);
}


static void ohm_fact_store_view_init (OhmFactStoreView * self) {
	self->priv = OHM_FACT_STORE_VIEW_GET_PRIVATE (self);
	self->priv->_listener = NULL;
	self->priv->_fact_store = NULL;
}


static void ohm_fact_store_view_dispose (GObject * obj) {
	OhmFactStoreView * self;
	self = OHM_FACT_STORE_VIEW (obj);
	(self->priv->_listener == NULL ? NULL : (self->priv->_listener = (g_object_unref (self->priv->_listener), NULL)));
	(self->priv->_fact_store == NULL ? NULL : (self->priv->_fact_store = (g_object_unref (self->priv->_fact_store), NULL)));
	(self->change_set == NULL ? NULL : (self->change_set = (g_object_unref (self->change_set), NULL)));
	G_OBJECT_CLASS (ohm_fact_store_view_parent_class)->dispose (obj);
}


GType ohm_fact_store_view_get_type (void) {
	static GType ohm_fact_store_view_type_id = 0;
	if (G_UNLIKELY (ohm_fact_store_view_type_id == 0)) {
		static const GTypeInfo g_define_type_info = { sizeof (OhmFactStoreViewClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) ohm_fact_store_view_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (OhmFactStoreView), 0, (GInstanceInitFunc) ohm_fact_store_view_init };
		ohm_fact_store_view_type_id = g_type_register_static (G_TYPE_OBJECT, "OhmFactStoreView", &g_define_type_info, 0);
	}
	return ohm_fact_store_view_type_id;
}



GType ohm_fact_store_event_get_type (void) {
	static GType ohm_fact_store_event_type_id = 0;
	if (G_UNLIKELY (ohm_fact_store_event_type_id == 0)) {
		static const GEnumValue values[] = {{OHM_FACT_STORE_EVENT_ADDED, "OHM_FACT_STORE_EVENT_ADDED", "added"}, {OHM_FACT_STORE_EVENT_REMOVED, "OHM_FACT_STORE_EVENT_REMOVED", "removed"}, {OHM_FACT_STORE_EVENT_UPDATED, "OHM_FACT_STORE_EVENT_UPDATED", "updated"}, {0, NULL, NULL}};
		ohm_fact_store_event_type_id = g_enum_register_static ("OhmFactStoreEvent", values);
	}
	return ohm_fact_store_event_type_id;
}


static void ohm_fact_store_class_init (OhmFactStoreClass * klass) {
	ohm_fact_store_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (OhmFactStorePrivate));
	G_OBJECT_CLASS (klass)->dispose = ohm_fact_store_dispose;
	g_signal_new ("inserted", OHM_TYPE_FACT_STORE, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, OHM_TYPE_FACT);
	g_signal_new ("removed", OHM_TYPE_FACT_STORE, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, OHM_TYPE_FACT);
	g_signal_new ("updated", OHM_TYPE_FACT_STORE, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_user_marshal_VOID__OBJECT_POINTER_POINTER, G_TYPE_NONE, 3, OHM_TYPE_FACT, G_TYPE_POINTER, G_TYPE_VALUE);
}


static void ohm_fact_store_init (OhmFactStore * self) {
	self->priv = OHM_FACT_STORE_GET_PRIVATE (self);
	self->priv->facts = NULL;
}


static void ohm_fact_store_dispose (GObject * obj) {
	OhmFactStore * self;
	self = OHM_FACT_STORE (obj);
	(self->priv->facts == NULL ? NULL : (self->priv->facts = (g_list_free (self->priv->facts), NULL)));
	G_OBJECT_CLASS (ohm_fact_store_parent_class)->dispose (obj);
}


GType ohm_fact_store_get_type (void) {
	static GType ohm_fact_store_type_id = 0;
	if (G_UNLIKELY (ohm_fact_store_type_id == 0)) {
		static const GTypeInfo g_define_type_info = { sizeof (OhmFactStoreClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) ohm_fact_store_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (OhmFactStore), 0, (GInstanceInitFunc) ohm_fact_store_init };
		ohm_fact_store_type_id = g_type_register_static (G_TYPE_OBJECT, "OhmFactStore", &g_define_type_info, 0);
	}
	return ohm_fact_store_type_id;
}


OhmRule* ohm_rule_new (OhmFactStore* fact_store) {
	GParameter * __params;
	GParameter * __params_it;
	OhmRule * self;
	g_return_val_if_fail (OHM_IS_FACT_STORE (fact_store), NULL);
	__params = g_new0 (GParameter, 1);
	__params_it = __params;
	__params_it->name = "fact-store";
	g_value_init (&__params_it->value, OHM_TYPE_FACT_STORE);
	g_value_set_object (&__params_it->value, fact_store);
	__params_it++;
	self = g_object_newv (OHM_TYPE_RULE, __params_it - __params, __params);
	while (__params_it > __params) {
		--__params_it;
		g_value_unset (&__params_it->value);
	}
	g_free (__params);
	return self;
}


void ohm_rule_fire (OhmRule* self) {
	g_return_if_fail (OHM_IS_RULE (self));
	g_signal_emit_by_name (G_OBJECT (self), "fired", self->priv->view->change_set);
	ohm_fact_store_change_set_reset (self->priv->view->change_set);
}


OhmFactStore* ohm_rule_get_fact_store (OhmRule* self) {
	g_return_val_if_fail (OHM_IS_RULE (self), NULL);
	return self->priv->_fact_store;
}


void ohm_rule_set_fact_store (OhmRule* self, OhmFactStore* value) {
	OhmFactStore* _tmp2;
	OhmFactStore* _tmp1;
	g_return_if_fail (OHM_IS_RULE (self));
	_tmp2 = NULL;
	_tmp1 = NULL;
	self->priv->_fact_store = (_tmp2 = (_tmp1 = value, (_tmp1 == NULL ? NULL : g_object_ref (_tmp1))), (self->priv->_fact_store == NULL ? NULL : (self->priv->_fact_store = (g_object_unref (self->priv->_fact_store), NULL))), _tmp2);
}


GSList* ohm_rule_get_interested (OhmRule* self) {
	g_return_val_if_fail (OHM_IS_RULE (self), NULL);
	return ohm_fact_store_view_get_interested (self->priv->view);
}


void ohm_rule_set_interested (OhmRule* self, GSList* value) {
	g_return_if_fail (OHM_IS_RULE (self));
	ohm_fact_store_view_set_interested (self->priv->view, value);
}


GSList* ohm_rule_get_provide (OhmRule* self) {
	g_return_val_if_fail (OHM_IS_RULE (self), NULL);
	return self->priv->_provide;
}


void ohm_rule_set_provide (OhmRule* self, GSList* value) {
	g_return_if_fail (OHM_IS_RULE (self));
	self->priv->_provide = value;
}


static GObject * ohm_rule_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	OhmRuleClass * klass;
	GObjectClass * parent_class;
	OhmRule * self;
	klass = OHM_RULE_CLASS (g_type_class_peek (OHM_TYPE_RULE));
	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);
	self = OHM_RULE (obj);
	{
		OhmFactStoreView* _tmp1;
		_tmp1 = NULL;
		self->priv->view = (_tmp1 = ohm_fact_store_new_view (ohm_rule_get_fact_store (self), G_OBJECT (self)), (self->priv->view == NULL ? NULL : (self->priv->view = (g_object_unref (self->priv->view), NULL))), _tmp1);
	}
	return obj;
}


static void ohm_rule_get_property (GObject * object, guint property_id, GValue * value, GParamSpec * pspec) {
	OhmRule * self;
	self = OHM_RULE (object);
	switch (property_id) {
		case OHM_RULE_FACT_STORE:
		g_value_set_object (value, ohm_rule_get_fact_store (self));
		break;
		case OHM_RULE_INTERESTED:
		g_value_set_pointer (value, ohm_rule_get_interested (self));
		break;
		case OHM_RULE_PROVIDE:
		g_value_set_pointer (value, ohm_rule_get_provide (self));
		break;
		default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}


static void ohm_rule_set_property (GObject * object, guint property_id, const GValue * value, GParamSpec * pspec) {
	OhmRule * self;
	self = OHM_RULE (object);
	switch (property_id) {
		case OHM_RULE_FACT_STORE:
		ohm_rule_set_fact_store (self, g_value_get_object (value));
		break;
		case OHM_RULE_INTERESTED:
		ohm_rule_set_interested (self, g_value_get_pointer (value));
		break;
		case OHM_RULE_PROVIDE:
		ohm_rule_set_provide (self, g_value_get_pointer (value));
		break;
		default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}


static void ohm_rule_class_init (OhmRuleClass * klass) {
	ohm_rule_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (OhmRulePrivate));
	G_OBJECT_CLASS (klass)->get_property = ohm_rule_get_property;
	G_OBJECT_CLASS (klass)->set_property = ohm_rule_set_property;
	G_OBJECT_CLASS (klass)->constructor = ohm_rule_constructor;
	G_OBJECT_CLASS (klass)->dispose = ohm_rule_dispose;
	g_object_class_install_property (G_OBJECT_CLASS (klass), OHM_RULE_FACT_STORE, g_param_spec_object ("fact-store", "fact-store", "fact-store", OHM_TYPE_FACT_STORE, G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));
	g_object_class_install_property (G_OBJECT_CLASS (klass), OHM_RULE_INTERESTED, g_param_spec_pointer ("interested", "interested", "interested", G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE));
	g_object_class_install_property (G_OBJECT_CLASS (klass), OHM_RULE_PROVIDE, g_param_spec_pointer ("provide", "provide", "provide", G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE));
	g_signal_new ("fired", OHM_TYPE_RULE, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, OHM_FACT_STORE_TYPE_CHANGE_SET);
}


static void ohm_rule_init (OhmRule * self) {
	self->priv = OHM_RULE_GET_PRIVATE (self);
}


static void ohm_rule_dispose (GObject * obj) {
	OhmRule * self;
	self = OHM_RULE (obj);
	(self->priv->_fact_store == NULL ? NULL : (self->priv->_fact_store = (g_object_unref (self->priv->_fact_store), NULL)));
	(self->priv->view == NULL ? NULL : (self->priv->view = (g_object_unref (self->priv->view), NULL)));
	G_OBJECT_CLASS (ohm_rule_parent_class)->dispose (obj);
}


GType ohm_rule_get_type (void) {
	static GType ohm_rule_type_id = 0;
	if (G_UNLIKELY (ohm_rule_type_id == 0)) {
		static const GTypeInfo g_define_type_info = { sizeof (OhmRuleClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) ohm_rule_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (OhmRule), 0, (GInstanceInitFunc) ohm_rule_init };
		ohm_rule_type_id = g_type_register_static (G_TYPE_OBJECT, "OhmRule", &g_define_type_info, 0);
	}
	return ohm_rule_type_id;
}


GValue ohm_value_from_string (const char* str) {
	GValue _tmp0 = {0};
	GValue value;
	0;
	value = (g_value_init (&_tmp0, G_TYPE_STRING), _tmp0);
	g_value_set_string (&value, str);
	return value;
}


GValue ohm_value_from_int (gint val) {
	GValue _tmp0 = {0};
	GValue value;
	value = (g_value_init (&_tmp0, G_TYPE_INT), _tmp0);
	g_value_set_int (&value, val);
	return value;
}


GValue ohm_value_from_structure (OhmStructure* val) {
	GValue _tmp0 = {0};
	GValue value;
	0;
	value = (g_value_init (&_tmp0, OHM_TYPE_STRUCTURE), _tmp0);
	g_value_set_object (&value, G_OBJECT (val));
	return value;
}


OhmStructure* ohm_value_get_structure (GValue* value) {
	OhmStructure* _tmp1;
	if (!G_VALUE_HOLDS (&(*value), OHM_TYPE_STRUCTURE)) {
		return NULL;
	}
	_tmp1 = NULL;
	return (_tmp1 = OHM_STRUCTURE (g_value_get_object (&(*value))), (_tmp1 == NULL ? NULL : g_object_ref (_tmp1)));
}



static void g_cclosure_user_marshal_VOID__OBJECT_POINTER_POINTER (GClosure * closure, GValue * return_value, guint n_param_values, const GValue * param_values, gpointer invocation_hint, gpointer marshal_data) {
	typedef void (*GMarshalFunc_VOID__OBJECT_POINTER_POINTER) (gpointer data1, gpointer arg_1, GQuark arg_2, gpointer arg_3, gpointer data2);
	register GMarshalFunc_VOID__OBJECT_POINTER_POINTER callback;
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
	callback = ((GMarshalFunc_VOID__OBJECT_POINTER_POINTER) (marshal_data ? marshal_data : cc->callback));

#if 0
	/* we get G_VALUE_HOLDS_POINTER runtime warning from
	 * g_value_get_pointer(param_values + 3) (ie. the new value) */
	callback (data1, g_value_get_object (param_values + 1), g_value_get_pointer (param_values + 2), g_value_get_pointer (param_values + 3), data2);
#else
	callback (data1,
		  g_value_get_object (param_values + 1),
		  g_value_get_pointer(param_values + 2),
		  NULL,
		  data2);
#endif
}



