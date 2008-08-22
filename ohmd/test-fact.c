
#include "test-fact.h"
#include <ohm/ohm-fact.h>
#include <stdlib.h>
#include <string.h>

/* stubs for leading-bleeding glob test 'framework' */
#define g_test_init(argc, argv, foo)
#define g_test_add_func(path, test) test()
#define g_test_run()


static void test_fact_structure_new (void);
static void test_fact_structure_name (void);
static void _structure_set_get (OhmStructure* s);
static void test_fact_structure_set_get (void);
static void test_fact_structure_free (void);
static void test_fact_structure_to_string (void);
static void test_fact_fact_new (void);
static void test_fact_fact_set_get (void);
static void test_fact_pattern_new (void);
static void test_fact_pattern_new_for_fact (void);
static void test_fact_pattern_set_get (void);
static void test_fact_pattern_free (void);
static void test_fact_pattern_match (void);
static void test_fact_pattern_match_instance (void);
static void test_fact_pattern_match_fields (void);
static void test_fact_pattern_match_free (void);
static void test_fact_store_new (void);
static void test_fact_store_insert (void);
static void test_fact_store_to_string (void);
static void test_fact_store_insert_remove (void);
static void test_fact_store_free (void);
static void test_fact_store_view_new (void);
static void test_fact_store_view_two (void);
static void test_fact_store_view_free (void);
static void test_fact_store_transaction_push_pop (void);
static void test_fact_store_transaction_push_and_watch (void);
static void test_fact_store_transaction_push_and_cancel (void);
static void test_fact_store_transaction_free (void);
static void _test_fact_structure_new_gcallback (void);
static void _test_fact_structure_name_gcallback (void);
static void _test_fact_structure_set_get_gcallback (void);
static void _test_fact_structure_free_gcallback (void);
static void _test_fact_structure_to_string_gcallback (void);
static void _test_fact_fact_new_gcallback (void);
static void _test_fact_fact_set_get_gcallback (void);
static void _test_fact_pattern_new_gcallback (void);
static void _test_fact_pattern_new_for_fact_gcallback (void);
static void _test_fact_pattern_set_get_gcallback (void);
static void _test_fact_pattern_free_gcallback (void);
static void _test_fact_pattern_match_gcallback (void);
static void _test_fact_pattern_match_fields_gcallback (void);
static void _test_fact_pattern_match_free_gcallback (void);
static void _test_fact_store_new_gcallback (void);
static void _test_fact_store_insert_gcallback (void);
static void _test_fact_store_to_string_gcallback (void);
static void _test_fact_store_insert_remove_gcallback (void);
static void _test_fact_store_free_gcallback (void);
static void _test_fact_store_view_new_gcallback (void);
static void _test_fact_store_view_two_gcallback (void);
static void _test_fact_store_view_free_gcallback (void);
static void _test_fact_store_transaction_push_pop_gcallback (void);
static void _test_fact_store_transaction_push_and_watch_gcallback (void);
static void _test_fact_store_transaction_push_and_cancel_gcallback (void);
static void _test_fact_store_transaction_free_gcallback (void);
static int _vala_strcmp0 (const char * str1, const char * str2);



/* void test_fact_pair_new () {
 // using struct
 // var p = Pair<string,int> ("yeayh!", 42);
 // var pp = Pair<string, Pair<string, int>> ("yeayh!", Pair<string,int> ("yeayh!", 42));
 var p = new Pair<string,int> ("yeayh!", 42);
 var pp = new Pair<string, Pair<string, int>> ("yeayh!", new Pair<string,int> ("yeayh!", 42));
 }*/
static void test_fact_structure_new (void) {
	void* p;
	p = NULL;
	{
		OhmStructure* s;
		s = ohm_structure_new ("org.freedesktop.ohm.test");
		p = s;
		g_object_add_weak_pointer (G_OBJECT (s), &p);
		(s == NULL ? NULL : (s = (g_object_unref (s), NULL)));
	}
	g_assert (p == NULL);
}


static void test_fact_structure_name (void) {
	OhmStructure* s;
	s = ohm_structure_new ("org.freedesktop.ohm.test");
	g_assert (_vala_strcmp0 (ohm_structure_get_name (s), "org.freedesktop.ohm.test") == 0);
	g_assert (ohm_structure_get_qname (s) == g_quark_from_string ("org.freedesktop.ohm.test"));
	(s == NULL ? NULL : (s = (g_object_unref (s), NULL)));
}


static void _structure_set_get (OhmStructure* s) {
	GValue* v;
	GValue* vq;
	g_return_if_fail (OHM_IS_STRUCTURE (s));
	g_assert (_vala_strcmp0 (ohm_structure_get_name (s), "org.freedesktop.ohm.test") == 0);
	g_assert (((void*) ohm_structure_get (s, "field1")) == NULL);
	ohm_structure_set (s, "field1", ohm_value_from_string ("test1"));
	ohm_structure_set (s, "field2", ohm_value_from_int (42));
	v = ((GValue*) ohm_structure_get (s, "field1"));
	g_assert (_vala_strcmp0 (g_value_get_string (v), "test1") == 0);
	v = ((GValue*) ohm_structure_get (s, "field2"));
	g_assert (g_value_get_int (v) == 42);
	vq = ((GValue*) ohm_structure_qget (s, g_quark_from_string ("field2")));
	g_assert (v == vq);
	ohm_structure_set (s, "field2", NULL);
	v = ((GValue*) ohm_structure_get (s, "field2"));
	g_assert (v == NULL);
}


static void test_fact_structure_set_get (void) {
	OhmStructure* s;
	s = ohm_structure_new ("org.freedesktop.ohm.test");
	_structure_set_get (s);
	(s == NULL ? NULL : (s = (g_object_unref (s), NULL)));
}


static void test_fact_structure_free (void) {
	{
		gint i;
		i = 0;
		for (; i < 20000; i++) {
			test_fact_structure_set_get ();
		}
	}
}


static void test_fact_structure_to_string (void) {
	OhmStructure* s;
	char* _tmp0;
	char* _tmp1;
	s = ohm_structure_new ("org.freedesktop.ohm.test");
	ohm_structure_set (s, "field1", ohm_value_from_string ("test1"));
	ohm_structure_set (s, "field2", ohm_value_from_int (42));
	_tmp0 = NULL;
	g_assert (_vala_strcmp0 ((_tmp0 = ohm_structure_to_string (s)), "org.freedesktop.ohm.test (field1 = \"test1\", field2 = 42)") == 0);
	_tmp0 = (g_free (_tmp0), NULL);
	ohm_structure_set (s, "field2", NULL);
	_tmp1 = NULL;
	g_assert (_vala_strcmp0 ((_tmp1 = ohm_structure_to_string (s)), "org.freedesktop.ohm.test (field1 = \"test1\")") == 0);
	_tmp1 = (g_free (_tmp1), NULL);
	(s == NULL ? NULL : (s = (g_object_unref (s), NULL)));
}


static void test_fact_fact_new (void) {
	void* p;
	p = NULL;
	{
		OhmFact* s;
		s = ohm_fact_new ("org.freedesktop.ohm.test");
		p = s;
		g_object_add_weak_pointer (G_OBJECT (s), &p);
		g_assert (ohm_fact_get_fact_store (s) == NULL);
		(s == NULL ? NULL : (s = (g_object_unref (s), NULL)));
	}
	g_assert (p == NULL);
}


static void test_fact_fact_set_get (void) {
	OhmFact* f;
	f = ohm_fact_new ("org.freedesktop.ohm.test");
	_structure_set_get (OHM_STRUCTURE (f));
	(f == NULL ? NULL : (f = (g_object_unref (f), NULL)));
}


static void test_fact_pattern_new (void) {
	void* p;
	p = NULL;
	{
		OhmPattern* s;
		s = ohm_pattern_new ("org.freedesktop.ohm.test");
		p = s;
		g_object_add_weak_pointer (G_OBJECT (s), &p);
		g_assert (ohm_pattern_get_view (s) == NULL);
		g_assert (ohm_pattern_get_fact (s) == NULL);
		(s == NULL ? NULL : (s = (g_object_unref (s), NULL)));
	}
	g_assert (p == NULL);
}


static void test_fact_pattern_new_for_fact (void) {
	void* pf;
	void* pp;
	pf = NULL;
	pp = NULL;
	{
		OhmFact* f;
		OhmPattern* p;
		OhmFact* _tmp0;
		f = ohm_fact_new ("org.test.match");
		p = ohm_pattern_new_for_fact (f);
		pf = f;
		pp = p;
		g_object_add_weak_pointer (G_OBJECT (f), &pf);
		g_object_add_weak_pointer (G_OBJECT (p), &pp);
		_tmp0 = NULL;
		f = (_tmp0 = NULL, (f == NULL ? NULL : (f = (g_object_unref (f), NULL))), _tmp0);
		g_assert (pf != NULL);
		g_assert (ohm_pattern_get_fact (p) != NULL);
		g_assert (_vala_strcmp0 (ohm_structure_get_name (OHM_STRUCTURE (ohm_pattern_get_fact (p))), "org.test.match") == 0);
		(f == NULL ? NULL : (f = (g_object_unref (f), NULL)));
		(p == NULL ? NULL : (p = (g_object_unref (p), NULL)));
	}
	g_assert (pf == NULL);
	g_assert (pp == NULL);
}


static void test_fact_pattern_set_get (void) {
	OhmPattern* f;
	f = ohm_pattern_new ("org.freedesktop.ohm.test");
	_structure_set_get (OHM_STRUCTURE (f));
	(f == NULL ? NULL : (f = (g_object_unref (f), NULL)));
}


static void test_fact_pattern_free (void) {
	{
		gint i;
		i = 0;
		for (; i < 1000; i++) {
			test_fact_pattern_new ();
			test_fact_pattern_new_for_fact ();
			test_fact_pattern_set_get ();
		}
	}
}


static void test_fact_pattern_match (void) {
	OhmPattern* p;
	OhmFact* match;
	OhmFact* match2;
	OhmFact* nomatch;
	OhmPatternMatch* _tmp0;
	OhmPatternMatch* _tmp1;
	OhmPatternMatch* _tmp2;
	p = ohm_pattern_new ("org.test.match");
	match = ohm_fact_new ("org.test.match");
	match2 = ohm_fact_new ("org.test.match");
	nomatch = ohm_fact_new ("org.test.nomatch");
	_tmp0 = NULL;
	g_assert ((_tmp0 = ohm_pattern_match (p, match, OHM_FACT_STORE_EVENT_LOOKUP)) != NULL);
	(_tmp0 == NULL ? NULL : (_tmp0 = (g_object_unref (_tmp0), NULL)));
	_tmp1 = NULL;
	g_assert ((_tmp1 = ohm_pattern_match (p, match2, OHM_FACT_STORE_EVENT_LOOKUP)) != NULL);
	(_tmp1 == NULL ? NULL : (_tmp1 = (g_object_unref (_tmp1), NULL)));
	_tmp2 = NULL;
	g_assert ((_tmp2 = ohm_pattern_match (p, nomatch, OHM_FACT_STORE_EVENT_LOOKUP)) == NULL);
	(_tmp2 == NULL ? NULL : (_tmp2 = (g_object_unref (_tmp2), NULL)));
	(p == NULL ? NULL : (p = (g_object_unref (p), NULL)));
	(match == NULL ? NULL : (match = (g_object_unref (match), NULL)));
	(match2 == NULL ? NULL : (match2 = (g_object_unref (match2), NULL)));
	(nomatch == NULL ? NULL : (nomatch = (g_object_unref (nomatch), NULL)));
}


static void test_fact_pattern_match_instance (void) {
	OhmFact* match;
	OhmFact* match2;
	OhmFact* nomatch;
	OhmPattern* pf;
	OhmPatternMatch* _tmp0;
	OhmPatternMatch* _tmp1;
	OhmPatternMatch* _tmp2;
	match = ohm_fact_new ("org.test.match");
	match2 = ohm_fact_new ("org.test.match");
	nomatch = ohm_fact_new ("org.test.nomatch");
	pf = ohm_pattern_new_for_fact (match);
	_tmp0 = NULL;
	g_assert ((_tmp0 = ohm_pattern_match (pf, match, OHM_FACT_STORE_EVENT_LOOKUP)) != NULL);
	(_tmp0 == NULL ? NULL : (_tmp0 = (g_object_unref (_tmp0), NULL)));
	_tmp1 = NULL;
	g_assert ((_tmp1 = ohm_pattern_match (pf, match2, OHM_FACT_STORE_EVENT_LOOKUP)) == NULL);
	(_tmp1 == NULL ? NULL : (_tmp1 = (g_object_unref (_tmp1), NULL)));
	_tmp2 = NULL;
	g_assert ((_tmp2 = ohm_pattern_match (pf, nomatch, OHM_FACT_STORE_EVENT_LOOKUP)) == NULL);
	(_tmp2 == NULL ? NULL : (_tmp2 = (g_object_unref (_tmp2), NULL)));
	(match == NULL ? NULL : (match = (g_object_unref (match), NULL)));
	(match2 == NULL ? NULL : (match2 = (g_object_unref (match2), NULL)));
	(nomatch == NULL ? NULL : (nomatch = (g_object_unref (nomatch), NULL)));
	(pf == NULL ? NULL : (pf = (g_object_unref (pf), NULL)));
}


static void test_fact_pattern_match_fields (void) {
	OhmPattern* p;
	OhmFact* match;
	OhmFact* match2;
	OhmFact* nomatch;
	OhmPatternMatch* _tmp0;
	OhmPatternMatch* _tmp1;
	OhmPatternMatch* _tmp2;
	OhmPatternMatch* _tmp3;
	OhmPatternMatch* m;
	OhmPatternMatch* _tmp4;
	OhmPatternMatch* _tmp5;
	OhmPatternMatch* _tmp6;
	OhmPatternMatch* _tmp7;
	p = ohm_pattern_new ("org.test.match");
	match = ohm_fact_new ("org.test.match");
	match2 = ohm_fact_new ("org.test.match");
	nomatch = ohm_fact_new ("org.test.nomatch");
	ohm_fact_set (match, "field1", ohm_value_from_string ("test1"));
	ohm_fact_set (match, "field2", ohm_value_from_int (42));
	ohm_fact_set (match2, "field2", ohm_value_from_int (42));
	_tmp0 = NULL;
	g_assert ((_tmp0 = ohm_pattern_match (p, match, OHM_FACT_STORE_EVENT_LOOKUP)) != NULL);
	(_tmp0 == NULL ? NULL : (_tmp0 = (g_object_unref (_tmp0), NULL)));
	_tmp1 = NULL;
	g_assert ((_tmp1 = ohm_pattern_match (p, match2, OHM_FACT_STORE_EVENT_LOOKUP)) != NULL);
	(_tmp1 == NULL ? NULL : (_tmp1 = (g_object_unref (_tmp1), NULL)));
	ohm_structure_set (OHM_STRUCTURE (p), "field2", ohm_value_from_int (42));
	_tmp2 = NULL;
	g_assert ((_tmp2 = ohm_pattern_match (p, match, OHM_FACT_STORE_EVENT_LOOKUP)) != NULL);
	(_tmp2 == NULL ? NULL : (_tmp2 = (g_object_unref (_tmp2), NULL)));
	_tmp3 = NULL;
	g_assert ((_tmp3 = ohm_pattern_match (p, match2, OHM_FACT_STORE_EVENT_LOOKUP)) != NULL);
	(_tmp3 == NULL ? NULL : (_tmp3 = (g_object_unref (_tmp3), NULL)));
	m = ohm_pattern_match (p, match2, OHM_FACT_STORE_EVENT_LOOKUP);
	g_assert (ohm_pattern_match_get_fact (m) == match2);
	g_assert (ohm_pattern_match_get_pattern (m) == p);
	g_assert (ohm_pattern_match_get_event (m) == OHM_FACT_STORE_EVENT_LOOKUP);
	ohm_structure_set (OHM_STRUCTURE (p), "field1", ohm_value_from_string ("test1"));
	_tmp4 = NULL;
	g_assert ((_tmp4 = ohm_pattern_match (p, match, OHM_FACT_STORE_EVENT_LOOKUP)) != NULL);
	(_tmp4 == NULL ? NULL : (_tmp4 = (g_object_unref (_tmp4), NULL)));
	_tmp5 = NULL;
	g_assert ((_tmp5 = ohm_pattern_match (p, match2, OHM_FACT_STORE_EVENT_LOOKUP)) == NULL);
	(_tmp5 == NULL ? NULL : (_tmp5 = (g_object_unref (_tmp5), NULL)));
	ohm_structure_set (OHM_STRUCTURE (p), "field1", ohm_value_from_string ("notest1"));
	_tmp6 = NULL;
	g_assert ((_tmp6 = ohm_pattern_match (p, match, OHM_FACT_STORE_EVENT_LOOKUP)) == NULL);
	(_tmp6 == NULL ? NULL : (_tmp6 = (g_object_unref (_tmp6), NULL)));
	_tmp7 = NULL;
	g_assert ((_tmp7 = ohm_pattern_match (p, match2, OHM_FACT_STORE_EVENT_LOOKUP)) == NULL);
	(_tmp7 == NULL ? NULL : (_tmp7 = (g_object_unref (_tmp7), NULL)));
	(p == NULL ? NULL : (p = (g_object_unref (p), NULL)));
	(match == NULL ? NULL : (match = (g_object_unref (match), NULL)));
	(match2 == NULL ? NULL : (match2 = (g_object_unref (match2), NULL)));
	(nomatch == NULL ? NULL : (nomatch = (g_object_unref (nomatch), NULL)));
	(m == NULL ? NULL : (m = (g_object_unref (m), NULL)));

	return;
	(void)test_fact_pattern_match_instance;
}


static void test_fact_pattern_match_free (void) {
	{
		gint i;
		i = 0;
		for (; i < 1000; i++) {
			test_fact_pattern_match_fields ();
		}
	}
}


static void test_fact_store_pattern_delete (void) {
#define FACT_NAME "com.nokia.ahoy"

	OhmFactStore     *fs;
	OhmFactStoreView *v1, *v2;
	OhmPattern       *p1, *p2;
	OhmFact          *f;

	fs = ohm_fact_store_new();
	v1 = ohm_fact_store_new_view(fs, NULL);
	v2 = ohm_fact_store_new_view(fs, NULL);
	
	p1 = ohm_pattern_new(FACT_NAME);
	p2 = ohm_pattern_new(FACT_NAME);
	
	ohm_fact_store_view_add(v1, OHM_STRUCTURE(p1));
	ohm_fact_store_view_add(v2, OHM_STRUCTURE(p2));

	g_object_unref(p1);
	g_object_unref(p2);

	f = ohm_fact_new(FACT_NAME);
	ohm_fact_store_insert(fs, f);

	ohm_fact_store_view_remove(v1, OHM_STRUCTURE(p1));
	g_object_unref(v1);
	
	f = ohm_fact_new(FACT_NAME);
	ohm_fact_store_insert(fs, f);
}


static void test_fact_store_new (void) {
	void* pfs;
	pfs = NULL;
	{
		OhmFactStore* fs;
		fs = ohm_fact_store_new ();
		pfs = fs;
		g_object_add_weak_pointer (G_OBJECT (fs), &pfs);
		g_assert (pfs != NULL);
		(fs == NULL ? NULL : (fs = (g_object_unref (fs), NULL)));
	}
	g_assert (pfs == NULL);
}


static void test_fact_store_insert (void) {
	void* p;
	p = NULL;
	{
		OhmFactStore* fs;
		OhmFact* fact;
		fs = ohm_fact_store_new ();
		fact = ohm_fact_new ("org.test.match");
		p = fact;
		g_object_add_weak_pointer (G_OBJECT (fact), &p);
		ohm_fact_store_insert (fs, fact);
		g_assert (p != NULL);
		(fs == NULL ? NULL : (fs = (g_object_unref (fs), NULL)));
		(fact == NULL ? NULL : (fact = (g_object_unref (fact), NULL)));
	}
	g_assert (p == NULL);
}


static void test_fact_store_to_string (void) {
	void* p;
	p = NULL;
	{
		OhmFactStore* fs;
		OhmFact* fact;
		OhmFact* _tmp0;
		char* s;
		fs = ohm_fact_store_new ();
		fact = ohm_fact_new ("org.test.match");
		p = fact;
		g_object_add_weak_pointer (G_OBJECT (fact), &p);
		ohm_fact_set (fact, "field1", ohm_value_from_string ("test1"));
		ohm_fact_set (fact, "field2", ohm_value_from_int (42));
		ohm_fact_store_insert (fs, fact);
		_tmp0 = NULL;
		fact = (_tmp0 = ohm_fact_new ("org.test.match"), (fact == NULL ? NULL : (fact = (g_object_unref (fact), NULL))), _tmp0);
		ohm_fact_set (fact, "field3", ohm_value_from_int (42));
		ohm_fact_store_insert (fs, fact);
		s = ohm_fact_store_to_string (fs);
		g_assert (strstr (s, "field1 = ") != NULL);
		g_assert (strstr (s, "field2 = 42") != NULL);
		g_assert (p != NULL);
		(fs == NULL ? NULL : (fs = (g_object_unref (fs), NULL)));
		(fact == NULL ? NULL : (fact = (g_object_unref (fact), NULL)));
		s = (g_free (s), NULL);
	}
	g_assert (p == NULL);
}


static void test_fact_store_insert_remove (void) {
	void* p;
	void* pfs;
	void* pf;
	p = NULL;
	pfs = NULL;
	pf = NULL;
	{
		OhmFactStore* fs;
		OhmFact* fact1;
		OhmFact* fact2;
		fs = ohm_fact_store_new ();
		pfs = fs;
		g_object_add_weak_pointer (G_OBJECT (fs), &pfs);
		fact1 = ohm_fact_new ("org.test.fact1");
		ohm_fact_set (fact1, "field1", ohm_value_from_string ("test1"));
		ohm_fact_set (fact1, "field2", ohm_value_from_int (42));
		p = fact1;
		g_object_add_weak_pointer (G_OBJECT (fact1), &p);
		fact2 = ohm_fact_new ("org.test.fact2");
		ohm_fact_set (fact2, "field1", ohm_value_from_string ("test2"));
		ohm_fact_set (fact2, "field2", ohm_value_from_int (42));
		/* should not complain, does not exists*/
		ohm_fact_store_remove (fs, fact1);
		/* add+remove the same fact*/
		ohm_fact_store_insert (fs, fact1);
		ohm_fact_store_insert (fs, fact1);
		ohm_fact_store_remove (fs, fact1);
		g_assert (g_slist_length (ohm_fact_store_get_facts_by_name (fs, "org.test.fact1")) == 0);
		ohm_fact_store_insert (fs, fact1);
		ohm_fact_store_insert (fs, fact2);
		g_assert (g_slist_length (ohm_fact_store_get_facts_by_name (fs, "org.test.fact1")) == 1);
		g_assert (g_slist_length (ohm_fact_store_get_facts_by_name (fs, "org.test.fact2")) == 1);
		ohm_fact_store_remove (fs, fact2);
		g_assert (g_slist_length (ohm_fact_store_get_facts_by_name (fs, "org.test.fact1")) == 1);
		g_assert (g_slist_length (ohm_fact_store_get_facts_by_name (fs, "org.test.fact2")) == 0);
		{
			gint i;
			i = 0;
			for (; i < 100; i++) {
				OhmFact* fact;
				char* _tmp1;
				char* _tmp0;
				fact = ohm_fact_new ("org.test.fact1");
				_tmp1 = NULL;
				_tmp0 = NULL;
				ohm_fact_set (fact, "alloc", ohm_value_from_string ((_tmp1 = g_strconcat ("test", (_tmp0 = g_strdup_printf ("%i", i)), NULL))));
				_tmp1 = (g_free (_tmp1), NULL);
				_tmp0 = (g_free (_tmp0), NULL);
				ohm_fact_store_insert (fs, fact);
				if (pf == NULL) {
					pf = fact;
					g_object_add_weak_pointer (G_OBJECT (fact), &pf);
				}
				(fact == NULL ? NULL : (fact = (g_object_unref (fact), NULL)));
			}
		}
		g_assert (g_slist_length (ohm_fact_store_get_facts_by_name (fs, "org.test.fact1")) == 101);
		g_assert (p != NULL);
		g_assert (pfs != NULL);
		g_assert (pf != NULL);
		(fs == NULL ? NULL : (fs = (g_object_unref (fs), NULL)));
		(fact1 == NULL ? NULL : (fact1 = (g_object_unref (fact1), NULL)));
		(fact2 == NULL ? NULL : (fact2 = (g_object_unref (fact2), NULL)));
	}
	g_assert (p == NULL);
	g_assert (pfs == NULL);
	g_assert (pf == NULL);
}


static void test_fact_store_free (void) {
	{
		gint i;
		i = 0;
		for (; i < 1000; i++) {
			test_fact_store_insert_remove ();
		}
	}
}


static void test_fact_store_view_new (void) {
	void* p;
	p = NULL;
	{
		OhmFactStore* fs;
		OhmFactStoreView* v;
		fs = ohm_fact_store_new ();
		p = fs;
		g_object_add_weak_pointer (G_OBJECT (fs), &p);
		v = ohm_fact_store_new_view (fs, NULL);
		g_assert (p != NULL);
		(fs == NULL ? NULL : (fs = (g_object_unref (fs), NULL)));
		(v == NULL ? NULL : (v = (g_object_unref (v), NULL)));
	}
	g_assert (p == NULL);
}


static void test_fact_store_view_two (void) {
	OhmFactStore* fs;
	OhmFactStoreView* v;
	OhmFactStoreView* v2;
	OhmPattern* _tmp0;
	OhmPattern* _tmp1;
	OhmFact* _tmp2;
	OhmFact* _tmp3;
	OhmFact* _tmp4;
	OhmFact* _tmp5;
	OhmFact* f;
	fs = ohm_fact_store_new ();
	v = ohm_fact_store_new_view (fs, NULL);
	v2 = ohm_fact_store_new_view (fs, NULL);
	_tmp0 = NULL;
	ohm_fact_store_view_add (v, OHM_STRUCTURE ((_tmp0 = ohm_pattern_new ("org.freedesktop.hello"))));
	(_tmp0 == NULL ? NULL : (_tmp0 = (g_object_unref (_tmp0), NULL)));
	_tmp1 = NULL;
	ohm_fact_store_view_add (v2, OHM_STRUCTURE ((_tmp1 = ohm_pattern_new ("org.freedesktop.hello"))));
	(_tmp1 == NULL ? NULL : (_tmp1 = (g_object_unref (_tmp1), NULL)));
	g_assert (g_slist_length (ohm_fact_store_change_set_get_matches (OHM_FACT_STORE_SIMPLE_VIEW (v)->change_set)) == 0);
	_tmp2 = NULL;
	ohm_fact_store_insert (fs, (_tmp2 = ohm_fact_new ("org.freedesktop.match2")));
	(_tmp2 == NULL ? NULL : (_tmp2 = (g_object_unref (_tmp2), NULL)));
	_tmp3 = NULL;
	ohm_fact_store_insert (fs, (_tmp3 = ohm_fact_new ("org.freedesktop.hello")));
	(_tmp3 == NULL ? NULL : (_tmp3 = (g_object_unref (_tmp3), NULL)));
	g_assert (g_slist_length (ohm_fact_store_change_set_get_matches (OHM_FACT_STORE_SIMPLE_VIEW (v)->change_set)) == 1);
	g_assert (g_slist_length (ohm_fact_store_change_set_get_matches (OHM_FACT_STORE_SIMPLE_VIEW (v2)->change_set)) == 1);
	_tmp4 = NULL;
	ohm_fact_store_insert (fs, (_tmp4 = ohm_fact_new ("org.freedesktop.hello")));
	(_tmp4 == NULL ? NULL : (_tmp4 = (g_object_unref (_tmp4), NULL)));
	g_assert (g_slist_length (ohm_fact_store_change_set_get_matches (OHM_FACT_STORE_SIMPLE_VIEW (v)->change_set)) == 2);
	g_assert (g_slist_length (ohm_fact_store_change_set_get_matches (OHM_FACT_STORE_SIMPLE_VIEW (v2)->change_set)) == 2);
	ohm_fact_store_change_set_reset (OHM_FACT_STORE_SIMPLE_VIEW (v)->change_set);
	g_assert (g_slist_length (ohm_fact_store_change_set_get_matches (OHM_FACT_STORE_SIMPLE_VIEW (v)->change_set)) == 0);
	_tmp5 = NULL;
	ohm_fact_store_insert (fs, (_tmp5 = ohm_fact_new ("org.freedesktop.hello")));
	(_tmp5 == NULL ? NULL : (_tmp5 = (g_object_unref (_tmp5), NULL)));
	g_assert (g_slist_length (ohm_fact_store_change_set_get_matches (OHM_FACT_STORE_SIMPLE_VIEW (v)->change_set)) != 0);
	g_assert (g_slist_length (ohm_fact_store_change_set_get_matches (OHM_FACT_STORE_SIMPLE_VIEW (v2)->change_set)) == 3);
	f = ohm_fact_new ("org.freedesktop.self");
	ohm_fact_store_change_set_reset (OHM_FACT_STORE_SIMPLE_VIEW (v2)->change_set);
	ohm_fact_store_view_add (v, OHM_STRUCTURE (f));
	ohm_fact_store_insert (fs, f);
	g_assert (g_slist_length (ohm_fact_store_change_set_get_matches (OHM_FACT_STORE_SIMPLE_VIEW (v)->change_set)) == 2);
	g_assert (g_slist_length (ohm_fact_store_change_set_get_matches (OHM_FACT_STORE_SIMPLE_VIEW (v2)->change_set)) == 0);
	(fs == NULL ? NULL : (fs = (g_object_unref (fs), NULL)));
	(v == NULL ? NULL : (v = (g_object_unref (v), NULL)));
	(v2 == NULL ? NULL : (v2 = (g_object_unref (v2), NULL)));
	(f == NULL ? NULL : (f = (g_object_unref (f), NULL)));
}


static void test_fact_store_view_free (void) {
	{
		gint i;
		i = 0;
		for (; i < 1000; i++) {
			test_fact_store_view_new ();
			test_fact_store_view_two ();
		}
	}
}


static void test_fact_store_transaction_push_pop (void) {
	OhmFactStore* fs;
	fs = ohm_fact_store_new ();
	ohm_fact_store_transaction_pop (fs, FALSE);
	ohm_fact_store_transaction_push (fs);
	ohm_fact_store_transaction_push (fs);
	ohm_fact_store_transaction_push (fs);
	ohm_fact_store_transaction_pop (fs, FALSE);
	ohm_fact_store_transaction_pop (fs, FALSE);
	{
		gint i;
		i = 0;
		for (; i < 100; i++) {
			ohm_fact_store_transaction_push (fs);
		}
	}
	{
		gint i;
		i = 0;
		for (; i < 100; i++) {
			ohm_fact_store_transaction_pop (fs, FALSE);
		}
	}
	ohm_fact_store_transaction_pop (fs, FALSE);
	(fs == NULL ? NULL : (fs = (g_object_unref (fs), NULL)));
}


static void test_fact_store_transaction_push_and_watch (void) {
	OhmFactStore* fs;
	OhmFact* fact;
	fs = ohm_fact_store_new ();
	ohm_fact_store_transaction_push (fs);
	fact = ohm_fact_new ("org.test.match");
	ohm_fact_set (fact, "field", ohm_value_from_int (42));
	ohm_fact_store_insert (fs, fact);
	g_assert (g_slist_length (ohm_fact_store_get_facts_by_name (fs, "org.test.match")) == 1);
	ohm_fact_set (fact, "field", ohm_value_from_int (43));
	ohm_fact_store_transaction_pop (fs, FALSE);
	g_assert (g_slist_length (ohm_fact_store_get_facts_by_name (fs, "org.test.match")) == 1);
	ohm_fact_store_transaction_push (fs);
	ohm_fact_store_transaction_push (fs);
	ohm_fact_store_remove (fs, fact);
	ohm_fact_store_transaction_pop (fs, FALSE);
	ohm_fact_store_transaction_pop (fs, FALSE);
	g_assert (g_slist_length (ohm_fact_store_get_facts_by_name (fs, "org.test.match")) == 0);
	(fs == NULL ? NULL : (fs = (g_object_unref (fs), NULL)));
	(fact == NULL ? NULL : (fact = (g_object_unref (fact), NULL)));
}


static void test_fact_store_transaction_push_and_cancel (void) {
	OhmFactStore* fs;
	OhmFactStoreView* v;
	OhmFactStoreView* v2;
	OhmPattern* _tmp0;
	OhmPattern* _tmp1;
	OhmFact* fact;
	GValue* val;
	fs = ohm_fact_store_new ();
	v = ohm_fact_store_new_view (fs, NULL);
	v2 = ohm_fact_store_new_view (fs, NULL);
	_tmp0 = NULL;
	ohm_fact_store_view_add (v, OHM_STRUCTURE ((_tmp0 = ohm_pattern_new ("org.test.match"))));
	(_tmp0 == NULL ? NULL : (_tmp0 = (g_object_unref (_tmp0), NULL)));
	_tmp1 = NULL;
	ohm_fact_store_view_add (v2, OHM_STRUCTURE ((_tmp1 = ohm_pattern_new ("org.freedesktop.hello"))));
	(_tmp1 == NULL ? NULL : (_tmp1 = (g_object_unref (_tmp1), NULL)));
	/* insertion*/
	ohm_fact_store_transaction_push (fs);
	{
		OhmFact* fact;
		fact = ohm_fact_new ("org.test.match");
		ohm_fact_set (fact, "field", ohm_value_from_int (42));
		ohm_fact_store_insert (fs, fact);
		g_assert (g_slist_length (ohm_fact_store_get_facts_by_name (fs, "org.test.match")) == 1);
		g_assert (g_slist_length (ohm_fact_store_change_set_get_matches (OHM_FACT_STORE_SIMPLE_VIEW (v)->change_set)) == 1);
		(fact == NULL ? NULL : (fact = (g_object_unref (fact), NULL)));
	}
	ohm_fact_store_transaction_pop (fs, TRUE);
	/* pop should remove from the view change_set*/
	g_assert (g_slist_length (ohm_fact_store_change_set_get_matches (OHM_FACT_STORE_SIMPLE_VIEW (v)->change_set)) == 0);
	/* and from the fact store*/
	g_assert (g_slist_length (ohm_fact_store_get_facts_by_name (fs, "org.test.match")) == 0);
	/* retract/remove*/
	fact = ohm_fact_new ("org.test.match");
	ohm_fact_set (fact, "field", ohm_value_from_int (42));
	ohm_fact_store_insert (fs, fact);
	ohm_fact_store_transaction_push (fs);
	{
		ohm_fact_store_remove (fs, fact);
		g_assert (g_slist_length (ohm_fact_store_get_facts_by_name (fs, "org.test.match")) == 0);
		g_assert (g_slist_length (ohm_fact_store_change_set_get_matches (OHM_FACT_STORE_SIMPLE_VIEW (v)->change_set)) == 2);
	}
	ohm_fact_store_transaction_pop (fs, TRUE);
	/* pop should remove from the view change_set*/
	g_assert (g_slist_length (ohm_fact_store_change_set_get_matches (OHM_FACT_STORE_SIMPLE_VIEW (v)->change_set)) == 1);
	/* and reinsert in the fact store*/
	g_assert (g_slist_length (ohm_fact_store_get_facts_by_name (fs, "org.test.match")) == 1);
	/* update*/
	ohm_fact_store_transaction_push (fs);
	{
		GValue* val;
		val = ((GValue*) ohm_fact_get (fact, "field"));
		g_assert (g_value_get_int (val) == 42);
		ohm_fact_set (fact, "field", ohm_value_from_int (43));
		val = ((GValue*) ohm_fact_get (fact, "field"));
		g_assert (g_value_get_int (val) == 43);
		g_assert (g_slist_length (ohm_fact_store_change_set_get_matches (OHM_FACT_STORE_SIMPLE_VIEW (v)->change_set)) == 2);
	}
	ohm_fact_store_transaction_pop (fs, TRUE);
	/* pop should remove from the view change_set*/
	g_assert (g_slist_length (ohm_fact_store_change_set_get_matches (OHM_FACT_STORE_SIMPLE_VIEW (v)->change_set)) == 1);
	val = ((GValue*) ohm_fact_get (fact, "field"));
	g_assert (g_value_get_int (val) == 42);
	(fs == NULL ? NULL : (fs = (g_object_unref (fs), NULL)));
	(v == NULL ? NULL : (v = (g_object_unref (v), NULL)));
	(v2 == NULL ? NULL : (v2 = (g_object_unref (v2), NULL)));
	(fact == NULL ? NULL : (fact = (g_object_unref (fact), NULL)));
}


static void test_fact_store_transaction_free (void) {
	{
		gint i;
		i = 1;
		for (; i < 1000; (i = i + 1)) {
			test_fact_store_transaction_push_and_cancel ();
		}
	}
}


static void _test_fact_structure_new_gcallback (void) {
	test_fact_structure_new ();
}


static void _test_fact_structure_name_gcallback (void) {
	test_fact_structure_name ();
}


static void _test_fact_structure_set_get_gcallback (void) {
	test_fact_structure_set_get ();
}


static void _test_fact_structure_free_gcallback (void) {
	test_fact_structure_free ();
}


static void _test_fact_structure_to_string_gcallback (void) {
	test_fact_structure_to_string ();
}


static void _test_fact_fact_new_gcallback (void) {
	test_fact_fact_new ();
}


static void _test_fact_fact_set_get_gcallback (void) {
	test_fact_fact_set_get ();
}


static void _test_fact_pattern_new_gcallback (void) {
	test_fact_pattern_new ();
}


static void _test_fact_pattern_new_for_fact_gcallback (void) {
	test_fact_pattern_new_for_fact ();
}


static void _test_fact_pattern_set_get_gcallback (void) {
	test_fact_pattern_set_get ();
}


static void _test_fact_pattern_free_gcallback (void) {
	test_fact_pattern_free ();
}


static void _test_fact_pattern_match_gcallback (void) {
	test_fact_pattern_match ();
}


static void _test_fact_pattern_match_fields_gcallback (void) {
	test_fact_pattern_match_fields ();
}


static void _test_fact_pattern_match_free_gcallback (void) {
	test_fact_pattern_match_free ();
}


static void _test_fact_store_pattern_delete_gcallback (void) {
	test_fact_store_pattern_delete ();
}


static void _test_fact_store_new_gcallback (void) {
	test_fact_store_new ();
}


static void _test_fact_store_insert_gcallback (void) {
	test_fact_store_insert ();
}


static void _test_fact_store_to_string_gcallback (void) {
	test_fact_store_to_string ();
}


static void _test_fact_store_insert_remove_gcallback (void) {
	test_fact_store_insert_remove ();
}


static void _test_fact_store_free_gcallback (void) {
	test_fact_store_free ();
}


static void _test_fact_store_view_new_gcallback (void) {
	test_fact_store_view_new ();
}


static void _test_fact_store_view_two_gcallback (void) {
	test_fact_store_view_two ();
}


static void _test_fact_store_view_free_gcallback (void) {
	test_fact_store_view_free ();
}


static void _test_fact_store_transaction_push_pop_gcallback (void) {
	test_fact_store_transaction_push_pop ();
}


static void _test_fact_store_transaction_push_and_watch_gcallback (void) {
	test_fact_store_transaction_push_and_watch ();
}


static void _test_fact_store_transaction_push_and_cancel_gcallback (void) {
	test_fact_store_transaction_push_and_cancel ();
}


static void _test_fact_store_transaction_free_gcallback (void) {
	test_fact_store_transaction_free ();
}


/* class RuleTest : Rule {
 public bool fired = false;
 public override void fire (FactStore.ChangeSet change_set) {
 this.fired = true;
 }
 }
 void test_fact_rule_new () {
 void* p = null;
 {
 var fs = new FactStore ();
 var r = Rule.fixme (typeof (RuleTest), fs);
 assert (r is RuleTest);
 p = r;
 r.add_weak_pointer (&p);
 assert (((RuleTest)r).fired == false);
 r.fire (r.view.change_set);
 assert (((RuleTest)r).fired == true);
 assert (p != null);
 }
 assert (p == null);
 }
 void test_fact_rule_free () {
 for (int i = 0; i < 1000; i++) {
 test_fact_rule_new ();
 }
 }*/
void _main (char** args, int args_length1) {
	/*mem_set_vtable (mem_profiler_table);
	Environment.atexit (mem_profile);*/
	g_test_init (&args_length1, &args, NULL);
	/*  Test.add_func ("/fact/pair/new", test_fact_pair_new);*/
	g_test_add_func ("/fact/structure/new", _test_fact_structure_new_gcallback);
	g_test_add_func ("/fact/structure/name", _test_fact_structure_name_gcallback);
	g_test_add_func ("/fact/structure/set_get", _test_fact_structure_set_get_gcallback);
	g_test_add_func ("/fact/structure/free", _test_fact_structure_free_gcallback);
	g_test_add_func ("/fact/structure/to_string", _test_fact_structure_to_string_gcallback);
	g_test_add_func ("/fact/fact/new", _test_fact_fact_new_gcallback);
	g_test_add_func ("/fact/fact/set_get", _test_fact_fact_set_get_gcallback);
	g_test_add_func ("/fact/pattern/new", _test_fact_pattern_new_gcallback);
	g_test_add_func ("/fact/pattern/new_for_fact", _test_fact_pattern_new_for_fact_gcallback);
	g_test_add_func ("/fact/pattern/set_get", _test_fact_pattern_set_get_gcallback);
	g_test_add_func ("/fact/pattern/free", _test_fact_pattern_free_gcallback);
	g_test_add_func ("/fact/pattern/match", _test_fact_pattern_match_gcallback);
	g_test_add_func ("/fact/pattern/match_instance", _test_fact_pattern_match_gcallback);
	g_test_add_func ("/fact/pattern/match_fields", _test_fact_pattern_match_fields_gcallback);
	g_test_add_func ("/fact/pattern/match_free", _test_fact_pattern_match_free_gcallback);
	g_test_add_func ("/fact/store/pattern/delete", _test_fact_store_pattern_delete_gcallback);
	g_test_add_func ("/fact/store/new", _test_fact_store_new_gcallback);
	g_test_add_func ("/fact/store/insert", _test_fact_store_insert_gcallback);
	g_test_add_func ("/fact/store/to_string", _test_fact_store_to_string_gcallback);
	g_test_add_func ("/fact/store/insert_remove", _test_fact_store_insert_remove_gcallback);
	g_test_add_func ("/fact/store/free", _test_fact_store_free_gcallback);
	g_test_add_func ("/fact/store/view/new", _test_fact_store_view_new_gcallback);
	g_test_add_func ("/fact/store/view/two", _test_fact_store_view_two_gcallback);
	g_test_add_func ("/fact/store/view/free", _test_fact_store_view_free_gcallback);
	g_test_add_func ("/fact/store/transaction/push_pop", _test_fact_store_transaction_push_pop_gcallback);
	g_test_add_func ("/fact/store/transaction/push_and_watch", _test_fact_store_transaction_push_and_watch_gcallback);
	g_test_add_func ("/fact/store/transaction/push_and_cancel", _test_fact_store_transaction_push_and_cancel_gcallback);
	g_test_add_func ("/fact/store/transaction/free", _test_fact_store_transaction_free_gcallback);
	/* Test.add_func ("/fact/rule/new", test_fact_rule_new);
	 Test.add_func ("/fact/rule/free", test_fact_rule_free);*/
	g_test_run ();
}


int main (int argc, char ** argv) {
	g_type_init ();
	_main (argv, argc);
	return 0;
}


static int _vala_strcmp0 (const char * str1, const char * str2) {
	if (str1 == NULL) {
		return -(str1 != str2);
	}
	if (str2 == NULL) {
		return (str1 != str2);
	}
	return strcmp (str1, str2);
}




