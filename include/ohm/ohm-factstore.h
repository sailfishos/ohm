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

#ifndef __OHM_FACTSTORE_H__
#define __OHM_FACTSTORE_H__

#define NEW_OHMFACT 1

#include <glib.h>
#include <glib-object.h>
#include <stdlib.h>
#include <string.h>

G_BEGIN_DECLS

typedef struct _OhmPair OhmPair;

#define OHM_TYPE_STRUCTURE (ohm_structure_get_type ())
#define OHM_STRUCTURE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), OHM_TYPE_STRUCTURE, OhmStructure))
#define OHM_STRUCTURE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), OHM_TYPE_STRUCTURE, OhmStructureClass))
#define OHM_IS_STRUCTURE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OHM_TYPE_STRUCTURE))
#define OHM_IS_STRUCTURE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), OHM_TYPE_STRUCTURE))
#define OHM_STRUCTURE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), OHM_TYPE_STRUCTURE, OhmStructureClass))

typedef struct _OhmStructure OhmStructure;
typedef struct _OhmStructureClass OhmStructureClass;
typedef struct _OhmStructurePrivate OhmStructurePrivate;

#define OHM_TYPE_PATTERN (ohm_pattern_get_type ())
#define OHM_PATTERN(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), OHM_TYPE_PATTERN, OhmPattern))
#define OHM_PATTERN_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), OHM_TYPE_PATTERN, OhmPatternClass))
#define OHM_IS_PATTERN(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OHM_TYPE_PATTERN))
#define OHM_IS_PATTERN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), OHM_TYPE_PATTERN))
#define OHM_PATTERN_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), OHM_TYPE_PATTERN, OhmPatternClass))

typedef struct _OhmPattern OhmPattern;
typedef struct _OhmPatternClass OhmPatternClass;
typedef struct _OhmPatternPrivate OhmPatternPrivate;

#define OHM_PATTERN_TYPE_MATCH (ohm_pattern_match_get_type ())
#define OHM_PATTERN_MATCH(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), OHM_PATTERN_TYPE_MATCH, OhmPatternMatch))
#define OHM_PATTERN_MATCH_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), OHM_PATTERN_TYPE_MATCH, OhmPatternMatchClass))
#define OHM_PATTERN_IS_MATCH(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OHM_PATTERN_TYPE_MATCH))
#define OHM_PATTERN_IS_MATCH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), OHM_PATTERN_TYPE_MATCH))
#define OHM_PATTERN_MATCH_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), OHM_PATTERN_TYPE_MATCH, OhmPatternMatchClass))

typedef struct _OhmPatternMatch OhmPatternMatch;
typedef struct _OhmPatternMatchClass OhmPatternMatchClass;
typedef struct _OhmPatternMatchPrivate OhmPatternMatchPrivate;

#define OHM_TYPE_FACT (ohm_fact_get_type ())
#define OHM_FACT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), OHM_TYPE_FACT, OhmFact))
#define OHM_FACT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), OHM_TYPE_FACT, OhmFactClass))
#define OHM_IS_FACT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OHM_TYPE_FACT))
#define OHM_IS_FACT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), OHM_TYPE_FACT))
#define OHM_FACT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), OHM_TYPE_FACT, OhmFactClass))

typedef struct _OhmFact OhmFact;
typedef struct _OhmFactClass OhmFactClass;
typedef struct _OhmFactPrivate OhmFactPrivate;

#define OHM_TYPE_FACT_STORE (ohm_fact_store_get_type ())
#define OHM_FACT_STORE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), OHM_TYPE_FACT_STORE, OhmFactStore))
#define OHM_FACT_STORE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), OHM_TYPE_FACT_STORE, OhmFactStoreClass))
#define OHM_IS_FACT_STORE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OHM_TYPE_FACT_STORE))
#define OHM_IS_FACT_STORE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), OHM_TYPE_FACT_STORE))
#define OHM_FACT_STORE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), OHM_TYPE_FACT_STORE, OhmFactStoreClass))

typedef struct _OhmFactStore OhmFactStore;
typedef struct _OhmFactStoreClass OhmFactStoreClass;
typedef struct _OhmFactStorePrivate OhmFactStorePrivate;

#define OHM_FACT_STORE_TYPE_CHANGE_SET (ohm_fact_store_change_set_get_type ())
#define OHM_FACT_STORE_CHANGE_SET(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), OHM_FACT_STORE_TYPE_CHANGE_SET, OhmFactStoreChangeSet))
#define OHM_FACT_STORE_CHANGE_SET_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), OHM_FACT_STORE_TYPE_CHANGE_SET, OhmFactStoreChangeSetClass))
#define OHM_FACT_STORE_IS_CHANGE_SET(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OHM_FACT_STORE_TYPE_CHANGE_SET))
#define OHM_FACT_STORE_IS_CHANGE_SET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), OHM_FACT_STORE_TYPE_CHANGE_SET))
#define OHM_FACT_STORE_CHANGE_SET_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), OHM_FACT_STORE_TYPE_CHANGE_SET, OhmFactStoreChangeSetClass))

typedef struct _OhmFactStoreChangeSet OhmFactStoreChangeSet;
typedef struct _OhmFactStoreChangeSetClass OhmFactStoreChangeSetClass;
typedef struct _OhmFactStoreChangeSetPrivate OhmFactStoreChangeSetPrivate;

#define OHM_FACT_STORE_TYPE_SIMPLE_VIEW (ohm_fact_store_simple_view_get_type ())
#define OHM_FACT_STORE_SIMPLE_VIEW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), OHM_FACT_STORE_TYPE_SIMPLE_VIEW, OhmFactStoreSimpleView))
#define OHM_FACT_STORE_SIMPLE_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), OHM_FACT_STORE_TYPE_SIMPLE_VIEW, OhmFactStoreSimpleViewClass))
#define OHM_FACT_STORE_IS_SIMPLE_VIEW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OHM_FACT_STORE_TYPE_SIMPLE_VIEW))
#define OHM_FACT_STORE_IS_SIMPLE_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), OHM_FACT_STORE_TYPE_SIMPLE_VIEW))
#define OHM_FACT_STORE_SIMPLE_VIEW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), OHM_FACT_STORE_TYPE_SIMPLE_VIEW, OhmFactStoreSimpleViewClass))

typedef struct _OhmFactStoreSimpleView OhmFactStoreSimpleView;
typedef struct _OhmFactStoreSimpleViewClass OhmFactStoreSimpleViewClass;
typedef struct _OhmFactStoreSimpleViewPrivate OhmFactStoreSimpleViewPrivate;

#define OHM_FACT_STORE_TYPE_TRANSACTION (ohm_fact_store_transaction_get_type ())
#define OHM_FACT_STORE_TRANSACTION(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), OHM_FACT_STORE_TYPE_TRANSACTION, OhmFactStoreTransaction))
#define OHM_FACT_STORE_TRANSACTION_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), OHM_FACT_STORE_TYPE_TRANSACTION, OhmFactStoreTransactionClass))
#define OHM_FACT_STORE_IS_TRANSACTION(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OHM_FACT_STORE_TYPE_TRANSACTION))
#define OHM_FACT_STORE_IS_TRANSACTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), OHM_FACT_STORE_TYPE_TRANSACTION))
#define OHM_FACT_STORE_TRANSACTION_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), OHM_FACT_STORE_TYPE_TRANSACTION, OhmFactStoreTransactionClass))

typedef struct _OhmFactStoreTransaction OhmFactStoreTransaction;
typedef struct _OhmFactStoreTransactionClass OhmFactStoreTransactionClass;
typedef struct _OhmFactStoreTransactionPrivate OhmFactStoreTransactionPrivate;
typedef struct _OhmFactStoreTransactionCOW OhmFactStoreTransactionCOW;

#define OHM_FACT_STORE_TYPE_VIEW (ohm_fact_store_view_get_type ())
#define OHM_FACT_STORE_VIEW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), OHM_FACT_STORE_TYPE_VIEW, OhmFactStoreView))
#define OHM_FACT_STORE_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), OHM_FACT_STORE_TYPE_VIEW, OhmFactStoreViewClass))
#define OHM_FACT_STORE_IS_VIEW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OHM_FACT_STORE_TYPE_VIEW))
#define OHM_FACT_STORE_IS_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), OHM_FACT_STORE_TYPE_VIEW))
#define OHM_FACT_STORE_VIEW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), OHM_FACT_STORE_TYPE_VIEW, OhmFactStoreViewClass))

typedef struct _OhmFactStoreView OhmFactStoreView;
typedef struct _OhmFactStoreViewClass OhmFactStoreViewClass;
typedef struct _OhmFactStoreViewPrivate OhmFactStoreViewPrivate;

#define OHM_FACT_STORE_TYPE_EVENT (ohm_fact_store_event_get_type ())

#define OHM_TYPE_RULE (ohm_rule_get_type ())
#define OHM_RULE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), OHM_TYPE_RULE, OhmRule))
#define OHM_RULE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), OHM_TYPE_RULE, OhmRuleClass))
#define OHM_IS_RULE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OHM_TYPE_RULE))
#define OHM_IS_RULE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), OHM_TYPE_RULE))
#define OHM_RULE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), OHM_TYPE_RULE, OhmRuleClass))

typedef struct _OhmRule OhmRule;
typedef struct _OhmRuleClass OhmRuleClass;
typedef struct _OhmRulePrivate OhmRulePrivate;

/**
 * OhmPair:
 * @first:  a pointer
 * @second:  a 2nd pointer
 *
 * Used to handle a pair of values, ala C++.
 **/
struct _OhmPair {
	gpointer first;
	gpointer second;
	GDestroyNotify first_destroy_func;
	GDestroyNotify second_destroy_func;
};

struct _OhmStructure {
	GObject parent_instance;
	OhmStructurePrivate * priv;
	GSList* fields;
	GQuark _name;
};

struct _OhmStructureClass {
	GObjectClass parent_class;
	void (*qset) (OhmStructure* self, GQuark field, GValue* value);
};

/**
 * OhmPattern:
 *
 * #OhmPattern is used to express a pattern to match against a #OhmFact.
 * It's usually used by the #OhmFactStoreView to declare interest in facts.
 *
 * See ohm_pattern_match() and #OhmPatternMatch for more information.
 **/
struct _OhmPattern {
	OhmStructure parent_instance;
	OhmPatternPrivate * priv;
};

struct _OhmPatternClass {
	OhmStructureClass parent_class;
};

/**
 * OhmPatternMatch:
 *
 * The result of a ohm_pattern_match () evaluation.
 *
 * See ohm_pattern_match() and #OhmPattern for more information.
 **/
struct _OhmPatternMatch {
	GObject parent_instance;
	OhmPatternMatchPrivate * priv;
};

struct _OhmPatternMatchClass {
	GObjectClass parent_class;
};

/**
 * OhmFact:
 *
 * #OhmFact inherits from #OhmStructure to represents a fact in a
 * #OhmFactStore, like:
 *
 * <informalexample>
 * <programlisting>
 * org.freedesktop.DBus.Object (path: "/a/path", connexion: ":1.2")
 * </programlisting>
 * </informalexample>
 * 
 * In the logical sense, "org.freedesktop.DBus.Object" would be a
 * predicate, and the fields are the named variables.
 **/
struct _OhmFact {
	OhmStructure parent_instance;
	OhmFactPrivate * priv;
};

struct _OhmFactClass {
	OhmStructureClass parent_class;
};

/**
 * OhmFactStore:
 *
 * This object serves as the knowledge base for all the known facts.
 *
 * You typically need to get the singleton with ohm_get_fact_store ().
 * Then, you can call either: 
 * <itemizedlist>
 * <listitem>
 * <para>
 * ohm_fact_store_insert () to insert a #OhmFact in the fact-store.
 * </para>
 * </listitem>
 * <listitem>
 * <para>
 * ohm_fact_store_remove () to remove a #OhmFact from the fact-store.
 * </para>
 * </listitem>
 * </itemizedlist>
 **/
struct _OhmFactStore {
	GObject parent_instance;
	OhmFactStorePrivate * priv;
	GQueue* transaction;
};

struct _OhmFactStoreClass {
	GObjectClass parent_class;
};

/**
 * OhmFactStoreChangeSet:
 *
 * This object serves as an aggregator of the changes.
 *
 **/
struct _OhmFactStoreChangeSet {
	GObject parent_instance;
	OhmFactStoreChangeSetPrivate * priv;
};

struct _OhmFactStoreChangeSetClass {
	GObjectClass parent_class;
};

/**
 * OhmFactStoreSimpleView:
 *
 * The base class to implement views.
 * You typically need #OhmFactStoreView only.
 *
 **/
struct _OhmFactStoreSimpleView {
	GObject parent_instance;
	OhmFactStoreSimpleViewPrivate * priv;
	OhmFactStoreChangeSet* change_set;
};

struct _OhmFactStoreSimpleViewClass {
	GObjectClass parent_class;
};

/**
 * OhmFactStoreTransaction:
 *
 * The class that represent transactions. Used internally.
 *
 **/
struct _OhmFactStoreTransaction {
	OhmFactStoreSimpleView parent_instance;
	OhmFactStoreTransactionPrivate * priv;
	GSList* matches;
	GSList* modifications;
};

struct _OhmFactStoreTransactionClass {
	OhmFactStoreSimpleViewClass parent_class;
};

struct _OhmFactStoreTransactionCOW {
	OhmFact* fact;
	gint event;
	GQuark field;
	GValue* value;
};

/**
 * OhmFactStoreView:
 *
 * A #OhmFactStoreView custom view.
 * 
 * A view is created with ohm_fact_store_new_view (). Use
 * ohm_fact_store_view_add () to listen for changes, and
 * ohm_fact_store_view_remove () to remove interest for
 * changes in this view.
 *
 **/
struct _OhmFactStoreView {
	OhmFactStoreSimpleView parent_instance;
	OhmFactStoreViewPrivate * priv;
	GSList* patterns;
};

struct _OhmFactStoreViewClass {
	OhmFactStoreSimpleViewClass parent_class;
};

typedef enum  {
	OHM_FACT_STORE_EVENT_ADDED,
	OHM_FACT_STORE_EVENT_REMOVED,
	OHM_FACT_STORE_EVENT_UPDATED,
	OHM_FACT_STORE_EVENT_LOOKUP
} OhmFactStoreEvent;

OhmPair* ohm_pair_new (gpointer first, gpointer second, 
		       GDestroyNotify first_destroy_func, GDestroyNotify second_destroy_func);
void ohm_pair_free (OhmPair* self);

OhmStructure* ohm_structure_new (const char* name);
void ohm_structure_qset (OhmStructure* self, GQuark field, GValue* value);
GValue* ohm_structure_qget (OhmStructure* self, GQuark field);
void ohm_structure_set (OhmStructure* self, const char* field_name, GValue* value);
GValue* ohm_structure_get (OhmStructure* self, const char* field_name);
char* ohm_structure_to_string (OhmStructure* self);
void ohm_structure_value_to_string (const GValue* src, GValue* dest);
GQuark ohm_structure_get_qname (OhmStructure* self);
const char* ohm_structure_get_name (OhmStructure* self);
void ohm_structure_set_name (OhmStructure* self, const char* value);
GType ohm_structure_get_type (void);

OhmPattern* ohm_pattern_new (const char* name);
OhmPattern* ohm_pattern_new_for_fact (OhmFact* fact);
OhmPatternMatch* ohm_pattern_match (OhmPattern* self, OhmFact* fact, OhmFactStoreEvent event);
OhmFactStoreView* ohm_pattern_get_view (OhmPattern* self);
void ohm_pattern_set_view (OhmPattern* self, OhmFactStoreView* value);
OhmFact* ohm_pattern_get_fact (OhmPattern* self);
void ohm_pattern_set_fact (OhmPattern* self, OhmFact* value);
OhmPatternMatch* ohm_pattern_match_new (OhmFact* fact, OhmPattern* pattern, OhmFactStoreEvent event);
char* ohm_pattern_match_to_string (OhmPatternMatch* self);
OhmFact* ohm_pattern_match_get_fact (OhmPatternMatch* self);
OhmPattern* ohm_pattern_match_get_pattern (OhmPatternMatch* self);
OhmFactStoreEvent ohm_pattern_match_get_event (OhmPatternMatch* self);
GType ohm_pattern_match_get_type (void);
GType ohm_pattern_get_type (void);

OhmFact* ohm_fact_new (const char* name);
GValue* ohm_fact_get (OhmFact* self, const char* field_name);
void ohm_fact_set (OhmFact* self, const char* field_name, GValue* value);
OhmFactStore* ohm_fact_get_fact_store (OhmFact* self);
GSList *ohm_fact_get_fields(OhmFact *self);
void ohm_fact_set_fact_store (OhmFact* self, OhmFactStore* value);
GType ohm_fact_get_type (void);

gboolean ohm_fact_store_insert (OhmFactStore* self, OhmFact* fact);
void ohm_fact_store_remove (OhmFactStore* self, OhmFact* fact);
void ohm_fact_store_update (OhmFactStore* self, OhmFact* fact, GQuark field, GValue* value);
GSList* ohm_fact_store_get_facts_by_quark (OhmFactStore* self, GQuark qname);
GSList* ohm_fact_store_get_facts_by_name (OhmFactStore* self, const char* name);
GSList* ohm_fact_store_get_facts_by_pattern (OhmFactStore* self, OhmPattern* pattern);
void ohm_fact_store_transaction_push (OhmFactStore* self);
void ohm_fact_store_transaction_pop (OhmFactStore* self, gboolean discard);
OhmFactStore* ohm_fact_store_new (void);
char* ohm_fact_store_to_string (OhmFactStore* self);
OhmFactStoreView* ohm_fact_store_new_view (OhmFactStore* self, GObject* listener);
OhmFactStoreView* ohm_fact_store_new_transparent_view (OhmFactStore* self, GObject* listener);
void ohm_fact_store_change_set_add_match (OhmFactStoreChangeSet* self, OhmPatternMatch* match);
void ohm_fact_store_change_set_remove_match (OhmFactStoreChangeSet* self, OhmPatternMatch* match);
void ohm_fact_store_change_set_reset (OhmFactStoreChangeSet* self);
char* ohm_fact_store_change_set_to_string (OhmFactStoreChangeSet* self);
OhmFactStoreChangeSet* ohm_fact_store_change_set_new (void);
GSList* ohm_fact_store_change_set_get_matches (OhmFactStoreChangeSet* self);
GType ohm_fact_store_change_set_get_type (void);

OhmFactStoreSimpleView* ohm_fact_store_simple_view_new (void);
GObject* ohm_fact_store_simple_view_get_listener (OhmFactStoreSimpleView* self);
void ohm_fact_store_simple_view_set_listener (OhmFactStoreSimpleView* self, GObject* value);
OhmFactStore* ohm_fact_store_simple_view_get_fact_store (OhmFactStoreSimpleView* self);
void ohm_fact_store_simple_view_set_fact_store (OhmFactStoreSimpleView* self, OhmFactStore* value);
GType ohm_fact_store_simple_view_get_type (void);

OhmFactStoreTransactionCOW* ohm_fact_store_transaction_cow_new (OhmFact* fact, OhmFactStoreEvent event, GQuark field, GValue* value);
void ohm_fact_store_transaction_cow_free (OhmFactStoreTransactionCOW* self);
GType ohm_fact_store_transaction_get_type (void);

void ohm_fact_store_view_add (OhmFactStoreView* self, OhmStructure* interest);
void ohm_fact_store_view_remove (OhmFactStoreView* self, OhmStructure* interest);
char* ohm_fact_store_view_to_string (OhmFactStoreView* self);
GType ohm_fact_store_view_get_type (void);
GType ohm_fact_store_event_get_type (void);
GType ohm_fact_store_get_type (void);

OhmFactStore* ohm_get_fact_store (void);
GValue* ohm_value_from_string (const char* str);
GValue* ohm_value_from_int (gint val);
GValue* ohm_value_from_structure (OhmStructure* val);
GValue* ohm_value_from_fact (OhmFact* val);
gint ohm_value_cmp (GValue* v1, GValue* v2);
OhmStructure* ohm_value_get_structure (const GValue* value);
OhmFact* ohm_value_get_fact (GValue* value);


G_END_DECLS

#endif
