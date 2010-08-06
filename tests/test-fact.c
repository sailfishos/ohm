#include <glib.h>
#include <glib-object.h>
#include <ohm/ohm-fact.h>
#include <stdlib.h>
#include <string.h>

#include <check.h>
#include <stdio.h>

/* stubs for leading-bleeding glob test 'framework' */
#define g_test_init(argc, argv, foo)
#define g_test_add_func(path, test) test()
#define g_test_run()


static void _structure_set_get(OhmStructure* s);
static int _vala_strcmp0(const char * str1, const char * str2);



/* void test_fact_pair_new () {
 // using struct
 // var p = Pair<string,int> ("yeayh!", 42);
 // var pp = Pair<string, Pair<string, int>> ("yeayh!", Pair<string,int> ("yeayh!", 42));
 var p = new Pair<string,int> ("yeayh!", 42);
 var pp = new Pair<string, Pair<string, int>> ("yeayh!", new Pair<string,int> ("yeayh!", 42));
 }*/
START_TEST (test_fact_structure_new)
{
	void* p;
    p = NULL;
    {
        OhmStructure* s;
        s = ohm_structure_new("org.freedesktop.ohm.test");
        p = s;
        g_object_add_weak_pointer(G_OBJECT(s), &p);
        (s == NULL ? NULL : (s = (g_object_unref(s), NULL)));
    }
    fail_unless(p == NULL);
}
END_TEST


START_TEST (test_fact_structure_name)
{
    OhmStructure* s;
    s = ohm_structure_new("org.freedesktop.ohm.test");
    fail_unless(_vala_strcmp0(ohm_structure_get_name(s), "org.freedesktop.ohm.test") == 0);
    fail_unless(ohm_structure_get_qname(s) == g_quark_from_string("org.freedesktop.ohm.test"));
    (s == NULL ? NULL : (s = (g_object_unref(s), NULL)));
}
END_TEST


static void _structure_set_get(OhmStructure* s)
{
    GValue* v;
    GValue* vq;
    if(!(OHM_IS_STRUCTURE(s)))  {
    	fail("");
    	return;
    }
    fail_unless(_vala_strcmp0(ohm_structure_get_name(s), "org.freedesktop.ohm.test") == 0);
    fail_unless(((void*) ohm_structure_get(s, "field1")) == NULL);
    ohm_structure_set(s, "field1", ohm_value_from_string("test1"));
    ohm_structure_set(s, "field2", ohm_value_from_int(42));
    v = ((GValue*) ohm_structure_get(s, "field1"));
    fail_unless(_vala_strcmp0(g_value_get_string(v), "test1") == 0);
    v = ((GValue*) ohm_structure_get(s, "field2"));
    fail_unless(g_value_get_int(v) == 42);
    vq = ((GValue*) ohm_structure_qget(s, g_quark_from_string("field2")));
    fail_unless(v == vq);
    ohm_structure_set(s, "field2", NULL);
    v = ((GValue*) ohm_structure_get(s, "field2"));
    fail_unless(v == NULL);
}


START_TEST (test_fact_structure_set_get)
{
    OhmStructure* s;
    s = ohm_structure_new("org.freedesktop.ohm.test");
    _structure_set_get(s);
    (s == NULL ? NULL : (s = (g_object_unref(s), NULL)));
}
END_TEST


START_TEST (test_fact_structure_free)
{
    test_fact_structure_set_get(_i);
}
END_TEST


START_TEST (test_fact_structure_to_string)
{
    OhmStructure* s;
    char* _tmp0;
    char* _tmp1;
    s = ohm_structure_new("org.freedesktop.ohm.test");
    ohm_structure_set(s, "field1", ohm_value_from_string("test1"));
    ohm_structure_set(s, "field2", ohm_value_from_int(42));
    _tmp0 = NULL;
    fail_unless(_vala_strcmp0((_tmp0 = ohm_structure_to_string(s)), "org.freedesktop.ohm.test (field1 = \"test1\", field2 = 42)") == 0);
    _tmp0 = (g_free(_tmp0), NULL);
    ohm_structure_set(s, "field2", NULL);
    _tmp1 = NULL;
    fail_unless(_vala_strcmp0((_tmp1 = ohm_structure_to_string(s)), "org.freedesktop.ohm.test (field1 = \"test1\")") == 0);
    _tmp1 = (g_free(_tmp1), NULL);
    (s == NULL ? NULL : (s = (g_object_unref(s), NULL)));
}
END_TEST


START_TEST (test_fact_fact_new)
{
    void* p;
    p = NULL;
    {
        OhmFact* s;
        s = ohm_fact_new("org.freedesktop.ohm.test");
        p = s;
        g_object_add_weak_pointer(G_OBJECT(s), &p);
        fail_unless(ohm_fact_get_fact_store(s) == NULL);
        (s == NULL ? NULL : (s = (g_object_unref(s), NULL)));
    }
    fail_unless(p == NULL);
}
END_TEST


START_TEST (test_fact_fact_set_get)
{
    OhmFact* f;
    f = ohm_fact_new("org.freedesktop.ohm.test");
    _structure_set_get(OHM_STRUCTURE(f));
    (f == NULL ? NULL : (f = (g_object_unref(f), NULL)));
}
END_TEST


START_TEST (test_fact_pattern_new)
{
    void* p;
    p = NULL;
    {
        OhmPattern* s;
        s = ohm_pattern_new("org.freedesktop.ohm.test");
        p = s;
        g_object_add_weak_pointer(G_OBJECT(s), &p);
        fail_unless(ohm_pattern_get_view(s) == NULL);
        fail_unless(ohm_pattern_get_fact(s) == NULL);
        (s == NULL ? NULL : (s = (g_object_unref(s), NULL)));
    }
    fail_unless(p == NULL);
}
END_TEST


START_TEST (test_fact_pattern_new_for_fact)
{
    void* pf;
    void* pp;
    pf = NULL;
    pp = NULL;
    {
        OhmFact* f;
        OhmPattern* p;
        OhmFact* _tmp0;
        f = ohm_fact_new("org.test.match");
        p = ohm_pattern_new_for_fact(f);
        pf = f;
        pp = p;
        g_object_add_weak_pointer(G_OBJECT(f), &pf);
        g_object_add_weak_pointer(G_OBJECT(p), &pp);
        _tmp0 = NULL;
        f = (_tmp0 = NULL, (f == NULL ? NULL : (f = (g_object_unref(f), NULL))), _tmp0);
        fail_unless(pf != NULL);
        fail_unless(ohm_pattern_get_fact(p) != NULL);
        fail_unless(_vala_strcmp0(ohm_structure_get_name(OHM_STRUCTURE(ohm_pattern_get_fact(p))), "org.test.match") == 0);
        (f == NULL ? NULL : (f = (g_object_unref(f), NULL)));
        (p == NULL ? NULL : (p = (g_object_unref(p), NULL)));
    }
    fail_unless(pf == NULL);
    fail_unless(pp == NULL);
}
END_TEST


START_TEST (test_fact_pattern_set_get)
{
    OhmPattern* f;
    f = ohm_pattern_new("org.freedesktop.ohm.test");
    _structure_set_get(OHM_STRUCTURE(f));
    (f == NULL ? NULL : (f = (g_object_unref(f), NULL)));
}
END_TEST


START_TEST (test_fact_pattern_free)
{
    test_fact_pattern_new(_i);
    test_fact_pattern_new_for_fact(_i);
    test_fact_pattern_set_get(_i);
}
END_TEST


START_TEST (test_fact_pattern_match)
{
    OhmPattern* p;
    OhmFact* match;
    OhmFact* match2;
    OhmFact* nomatch;
    OhmPatternMatch* _tmp0;
    OhmPatternMatch* _tmp1;
    OhmPatternMatch* _tmp2;
    p = ohm_pattern_new("org.test.match");
    match = ohm_fact_new("org.test.match");
    match2 = ohm_fact_new("org.test.match");
    nomatch = ohm_fact_new("org.test.nomatch");
    _tmp0 = NULL;
    fail_unless((_tmp0 = ohm_pattern_match(p, match, OHM_FACT_STORE_EVENT_LOOKUP)) != NULL);
    (_tmp0 == NULL ? NULL : (_tmp0 = (g_object_unref(_tmp0), NULL)));
    _tmp1 = NULL;
    fail_unless((_tmp1 = ohm_pattern_match(p, match2, OHM_FACT_STORE_EVENT_LOOKUP)) != NULL);
    (_tmp1 == NULL ? NULL : (_tmp1 = (g_object_unref(_tmp1), NULL)));
    _tmp2 = NULL;
    fail_unless((_tmp2 = ohm_pattern_match(p, nomatch, OHM_FACT_STORE_EVENT_LOOKUP)) == NULL);
    (_tmp2 == NULL ? NULL : (_tmp2 = (g_object_unref(_tmp2), NULL)));
    (p == NULL ? NULL : (p = (g_object_unref(p), NULL)));
    (match == NULL ? NULL : (match = (g_object_unref(match), NULL)));
    (match2 == NULL ? NULL : (match2 = (g_object_unref(match2), NULL)));
    (nomatch == NULL ? NULL : (nomatch = (g_object_unref(nomatch), NULL)));
}
END_TEST


START_TEST (test_fact_pattern_match_instance)
{
    OhmFact* match;
    OhmFact* match2;
    OhmFact* nomatch;
    OhmPattern* pf;
    OhmPatternMatch* _tmp0;
    OhmPatternMatch* _tmp1;
    OhmPatternMatch* _tmp2;
    match = ohm_fact_new("org.test.match");
    match2 = ohm_fact_new("org.test.match");
    nomatch = ohm_fact_new("org.test.nomatch");
    pf = ohm_pattern_new_for_fact(match);
    _tmp0 = NULL;
    fail_unless((_tmp0 = ohm_pattern_match(pf, match, OHM_FACT_STORE_EVENT_LOOKUP)) != NULL);
    (_tmp0 == NULL ? NULL : (_tmp0 = (g_object_unref(_tmp0), NULL)));
    _tmp1 = NULL;
    fail_unless((_tmp1 = ohm_pattern_match(pf, match2, OHM_FACT_STORE_EVENT_LOOKUP)) == NULL);
    (_tmp1 == NULL ? NULL : (_tmp1 = (g_object_unref(_tmp1), NULL)));
    _tmp2 = NULL;
    fail_unless((_tmp2 = ohm_pattern_match(pf, nomatch, OHM_FACT_STORE_EVENT_LOOKUP)) == NULL);
    (_tmp2 == NULL ? NULL : (_tmp2 = (g_object_unref(_tmp2), NULL)));
    (match == NULL ? NULL : (match = (g_object_unref(match), NULL)));
    (match2 == NULL ? NULL : (match2 = (g_object_unref(match2), NULL)));
    (nomatch == NULL ? NULL : (nomatch = (g_object_unref(nomatch), NULL)));
    (pf == NULL ? NULL : (pf = (g_object_unref(pf), NULL)));
}
END_TEST


START_TEST (test_fact_pattern_match_fields)
{
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
    p = ohm_pattern_new("org.test.match");
    match = ohm_fact_new("org.test.match");
    match2 = ohm_fact_new("org.test.match");
    nomatch = ohm_fact_new("org.test.nomatch");
    ohm_fact_set(match, "field1", ohm_value_from_string("test1"));
    ohm_fact_set(match, "field2", ohm_value_from_int(42));
    ohm_fact_set(match2, "field2", ohm_value_from_int(42));
    _tmp0 = NULL;
    fail_unless((_tmp0 = ohm_pattern_match(p, match, OHM_FACT_STORE_EVENT_LOOKUP)) != NULL);
    (_tmp0 == NULL ? NULL : (_tmp0 = (g_object_unref(_tmp0), NULL)));
    _tmp1 = NULL;
    fail_unless((_tmp1 = ohm_pattern_match(p, match2, OHM_FACT_STORE_EVENT_LOOKUP)) != NULL);
    (_tmp1 == NULL ? NULL : (_tmp1 = (g_object_unref(_tmp1), NULL)));
    ohm_structure_set(OHM_STRUCTURE(p), "field2", ohm_value_from_int(42));
    _tmp2 = NULL;
    fail_unless((_tmp2 = ohm_pattern_match(p, match, OHM_FACT_STORE_EVENT_LOOKUP)) != NULL);
    (_tmp2 == NULL ? NULL : (_tmp2 = (g_object_unref(_tmp2), NULL)));
    _tmp3 = NULL;
    fail_unless((_tmp3 = ohm_pattern_match(p, match2, OHM_FACT_STORE_EVENT_LOOKUP)) != NULL);
    (_tmp3 == NULL ? NULL : (_tmp3 = (g_object_unref(_tmp3), NULL)));
    m = ohm_pattern_match(p, match2, OHM_FACT_STORE_EVENT_LOOKUP);
    fail_unless(ohm_pattern_match_get_fact(m) == match2);
    fail_unless(ohm_pattern_match_get_pattern(m) == p);
    fail_unless(ohm_pattern_match_get_event(m) == OHM_FACT_STORE_EVENT_LOOKUP);
    ohm_structure_set(OHM_STRUCTURE(p), "field1", ohm_value_from_string("test1"));
    _tmp4 = NULL;
    fail_unless((_tmp4 = ohm_pattern_match(p, match, OHM_FACT_STORE_EVENT_LOOKUP)) != NULL);
    (_tmp4 == NULL ? NULL : (_tmp4 = (g_object_unref(_tmp4), NULL)));
    _tmp5 = NULL;
    fail_unless((_tmp5 = ohm_pattern_match(p, match2, OHM_FACT_STORE_EVENT_LOOKUP)) == NULL);
    (_tmp5 == NULL ? NULL : (_tmp5 = (g_object_unref(_tmp5), NULL)));
    ohm_structure_set(OHM_STRUCTURE(p), "field1", ohm_value_from_string("notest1"));
    _tmp6 = NULL;
    fail_unless((_tmp6 = ohm_pattern_match(p, match, OHM_FACT_STORE_EVENT_LOOKUP)) == NULL);
    (_tmp6 == NULL ? NULL : (_tmp6 = (g_object_unref(_tmp6), NULL)));
    _tmp7 = NULL;
    fail_unless((_tmp7 = ohm_pattern_match(p, match2, OHM_FACT_STORE_EVENT_LOOKUP)) == NULL);
    (_tmp7 == NULL ? NULL : (_tmp7 = (g_object_unref(_tmp7), NULL)));
    (p == NULL ? NULL : (p = (g_object_unref(p), NULL)));
    (match == NULL ? NULL : (match = (g_object_unref(match), NULL)));
    (match2 == NULL ? NULL : (match2 = (g_object_unref(match2), NULL)));
    (nomatch == NULL ? NULL : (nomatch = (g_object_unref(nomatch), NULL)));
    (m == NULL ? NULL : (m = (g_object_unref(m), NULL)));

    return;
    (void)test_fact_pattern_match_instance;
}
END_TEST


START_TEST (test_fact_pattern_match_free)
{
    test_fact_pattern_match_fields(_i);
}
END_TEST


START_TEST (test_fact_store_pattern_delete)
{
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
END_TEST


START_TEST (test_fact_store_new)
{
    void* pfs;
    pfs = NULL;
    {
        OhmFactStore* fs;
        fs = ohm_fact_store_new();
        pfs = fs;
        g_object_add_weak_pointer(G_OBJECT(fs), &pfs);
        fail_unless(pfs != NULL);
        (fs == NULL ? NULL : (fs = (g_object_unref(fs), NULL)));
    }
    fail_unless(pfs == NULL);
}
END_TEST


START_TEST (test_fact_store_insert)
{
    void* p;
    p = NULL;
    {
        OhmFactStore* fs;
        OhmFact* fact;
        fs = ohm_fact_store_new();
        fact = ohm_fact_new("org.test.match");
        p = fact;
        g_object_add_weak_pointer(G_OBJECT(fact), &p);
        ohm_fact_store_insert(fs, fact);
        fail_unless(p != NULL);
        (fs == NULL ? NULL : (fs = (g_object_unref(fs), NULL)));
        (fact == NULL ? NULL : (fact = (g_object_unref(fact), NULL)));
    }
    fail_unless(p == NULL);
}
END_TEST


START_TEST (test_fact_store_to_string)
{
    void* p;
    p = NULL;
    {
        OhmFactStore* fs;
        OhmFact* fact;
        OhmFact* _tmp0;
        char* s;
        fs = ohm_fact_store_new();
        fact = ohm_fact_new("org.test.match");
        p = fact;
        g_object_add_weak_pointer(G_OBJECT(fact), &p);
        ohm_fact_set(fact, "field1", ohm_value_from_string("test1"));
        ohm_fact_set(fact, "field2", ohm_value_from_int(42));
        ohm_fact_store_insert(fs, fact);
        _tmp0 = NULL;
        fact = (_tmp0 = ohm_fact_new("org.test.match"), (fact == NULL ? NULL : (fact = (g_object_unref(fact), NULL))), _tmp0);
        ohm_fact_set(fact, "field3", ohm_value_from_int(42));
        ohm_fact_store_insert(fs, fact);
        s = ohm_fact_store_to_string(fs);
        fail_unless(strstr(s, "field1 = ") != NULL);
        fail_unless(strstr(s, "field2 = 42") != NULL);
        fail_unless(p != NULL);
        (fs == NULL ? NULL : (fs = (g_object_unref(fs), NULL)));
        (fact == NULL ? NULL : (fact = (g_object_unref(fact), NULL)));
        s = (g_free(s), NULL);
    }
    fail_unless(p == NULL);
}
END_TEST


START_TEST (test_fact_store_insert_remove)
{
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
        fs = ohm_fact_store_new();
        pfs = fs;
        g_object_add_weak_pointer(G_OBJECT(fs), &pfs);
        fact1 = ohm_fact_new("org.test.fact1");
        ohm_fact_set(fact1, "field1", ohm_value_from_string("test1"));
        ohm_fact_set(fact1, "field2", ohm_value_from_int(42));
        p = fact1;
        g_object_add_weak_pointer(G_OBJECT(fact1), &p);
        fact2 = ohm_fact_new("org.test.fact2");
        ohm_fact_set(fact2, "field1", ohm_value_from_string("test2"));
        ohm_fact_set(fact2, "field2", ohm_value_from_int(42));
        /* should not complain, does not exists*/
        ohm_fact_store_remove(fs, fact1);
        /* add+remove the same fact*/
        ohm_fact_store_insert(fs, fact1);
        ohm_fact_store_insert(fs, fact1);
        ohm_fact_store_remove(fs, fact1);
        fail_unless(g_slist_length(ohm_fact_store_get_facts_by_name(fs, "org.test.fact1")) == 0);
        ohm_fact_store_insert(fs, fact1);
        ohm_fact_store_insert(fs, fact2);
        fail_unless(g_slist_length(ohm_fact_store_get_facts_by_name(fs, "org.test.fact1")) == 1);
        fail_unless(g_slist_length(ohm_fact_store_get_facts_by_name(fs, "org.test.fact2")) == 1);
        ohm_fact_store_remove(fs, fact2);
        fail_unless(g_slist_length(ohm_fact_store_get_facts_by_name(fs, "org.test.fact1")) == 1);
        fail_unless(g_slist_length(ohm_fact_store_get_facts_by_name(fs, "org.test.fact2")) == 0);
        {
            gint i;
            i = 0;
            for (; i < 100; i++) {
                OhmFact* fact;
                char* _tmp1;
                char* _tmp0;
                fact = ohm_fact_new("org.test.fact1");
                _tmp1 = NULL;
                _tmp0 = NULL;
                ohm_fact_set(fact, "alloc", ohm_value_from_string((_tmp1 = g_strconcat("test", (_tmp0 = g_strdup_printf("%i", i)), NULL))));
                _tmp1 = (g_free(_tmp1), NULL);
                _tmp0 = (g_free(_tmp0), NULL);
                ohm_fact_store_insert(fs, fact);
                if (pf == NULL) {
                    pf = fact;
                    g_object_add_weak_pointer(G_OBJECT(fact), &pf);
                }
                (fact == NULL ? NULL : (fact = (g_object_unref(fact), NULL)));
            }
        }
        fail_unless(g_slist_length(ohm_fact_store_get_facts_by_name(fs, "org.test.fact1")) == 101);
        fail_unless(p != NULL);
        fail_unless(pfs != NULL);
        fail_unless(pf != NULL);
        (fs == NULL ? NULL : (fs = (g_object_unref(fs), NULL)));
        (fact1 == NULL ? NULL : (fact1 = (g_object_unref(fact1), NULL)));
        (fact2 == NULL ? NULL : (fact2 = (g_object_unref(fact2), NULL)));
    }
    fail_unless(p == NULL);
    fail_unless(pfs == NULL);
    fail_unless(pf == NULL);
}
END_TEST


START_TEST (test_fact_store_free)
{
    test_fact_store_insert_remove(_i);
}
END_TEST


START_TEST (test_fact_store_view_new)
{
    void* p;
    p = NULL;
    {
        OhmFactStore* fs;
        OhmFactStoreView* v;
        fs = ohm_fact_store_new();
        p = fs;
        g_object_add_weak_pointer(G_OBJECT(fs), &p);
        v = ohm_fact_store_new_view(fs, NULL);
        fail_unless(p != NULL);
        (fs == NULL ? NULL : (fs = (g_object_unref(fs), NULL)));
        (v == NULL ? NULL : (v = (g_object_unref(v), NULL)));
    }
    fail_unless(p == NULL);
}
END_TEST


START_TEST (test_fact_store_view_two)
{
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
    fs = ohm_fact_store_new();
    v = ohm_fact_store_new_view(fs, NULL);
    v2 = ohm_fact_store_new_view(fs, NULL);
    _tmp0 = NULL;
    ohm_fact_store_view_add(v, OHM_STRUCTURE((_tmp0 = ohm_pattern_new("org.freedesktop.hello"))));
    (_tmp0 == NULL ? NULL : (_tmp0 = (g_object_unref(_tmp0), NULL)));
    _tmp1 = NULL;
    ohm_fact_store_view_add(v2, OHM_STRUCTURE((_tmp1 = ohm_pattern_new("org.freedesktop.hello"))));
    (_tmp1 == NULL ? NULL : (_tmp1 = (g_object_unref(_tmp1), NULL)));
    fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(v)->change_set)) == 0);
    _tmp2 = NULL;
    ohm_fact_store_insert(fs, (_tmp2 = ohm_fact_new("org.freedesktop.match2")));
    (_tmp2 == NULL ? NULL : (_tmp2 = (g_object_unref(_tmp2), NULL)));
    _tmp3 = NULL;
    ohm_fact_store_insert(fs, (_tmp3 = ohm_fact_new("org.freedesktop.hello")));
    (_tmp3 == NULL ? NULL : (_tmp3 = (g_object_unref(_tmp3), NULL)));
    fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(v)->change_set)) == 1);
    fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(v2)->change_set)) == 1);
    _tmp4 = NULL;
    ohm_fact_store_insert(fs, (_tmp4 = ohm_fact_new("org.freedesktop.hello")));
    (_tmp4 == NULL ? NULL : (_tmp4 = (g_object_unref(_tmp4), NULL)));
    fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(v)->change_set)) == 2);
    fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(v2)->change_set)) == 2);
    ohm_fact_store_change_set_reset(OHM_FACT_STORE_SIMPLE_VIEW(v)->change_set);
    fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(v)->change_set)) == 0);
    _tmp5 = NULL;
    ohm_fact_store_insert(fs, (_tmp5 = ohm_fact_new("org.freedesktop.hello")));
    (_tmp5 == NULL ? NULL : (_tmp5 = (g_object_unref(_tmp5), NULL)));
    fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(v)->change_set)) != 0);
    fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(v2)->change_set)) == 3);
    f = ohm_fact_new("org.freedesktop.self");
    ohm_fact_store_change_set_reset(OHM_FACT_STORE_SIMPLE_VIEW(v2)->change_set);
    ohm_fact_store_view_add(v, OHM_STRUCTURE(f));
    ohm_fact_store_insert(fs, f);
    fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(v)->change_set)) == 2);
    fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(v2)->change_set)) == 0);
    (fs == NULL ? NULL : (fs = (g_object_unref(fs), NULL)));
    (v == NULL ? NULL : (v = (g_object_unref(v), NULL)));
    (v2 == NULL ? NULL : (v2 = (g_object_unref(v2), NULL)));
    (f == NULL ? NULL : (f = (g_object_unref(f), NULL)));
}
END_TEST


START_TEST (test_fact_store_view_free)
{
    test_fact_store_view_new(_i);
    test_fact_store_view_two(_i);
}
END_TEST


START_TEST (test_fact_store_transaction_push_pop)
{
    OhmFactStore* fs;
    fs = ohm_fact_store_new();
    ohm_fact_store_transaction_pop(fs, FALSE);
    ohm_fact_store_transaction_push(fs);
    ohm_fact_store_transaction_push(fs);
    ohm_fact_store_transaction_push(fs);
    ohm_fact_store_transaction_pop(fs, FALSE);
    ohm_fact_store_transaction_pop(fs, FALSE);
    {
        gint i;
        i = 0;
        for (; i < 100; i++) {
            ohm_fact_store_transaction_push(fs);
        }
    }
    {
        gint i;
        i = 0;
        for (; i < 100; i++) {
            ohm_fact_store_transaction_pop(fs, FALSE);
        }
    }
    ohm_fact_store_transaction_pop(fs, FALSE);
    (fs == NULL ? NULL : (fs = (g_object_unref(fs), NULL)));
}
END_TEST


START_TEST (test_fact_store_transaction_push_and_watch)
{
    OhmFactStore* fs;
    OhmFact* fact;
    fs = ohm_fact_store_new();
    ohm_fact_store_transaction_push(fs);
    fact = ohm_fact_new("org.test.match");
    ohm_fact_set(fact, "field", ohm_value_from_int(42));
    ohm_fact_store_insert(fs, fact);
    fail_unless(g_slist_length(ohm_fact_store_get_facts_by_name(fs, "org.test.match")) == 1);
    ohm_fact_set(fact, "field", ohm_value_from_int(43));
    ohm_fact_store_transaction_pop(fs, FALSE);
    fail_unless(g_slist_length(ohm_fact_store_get_facts_by_name(fs, "org.test.match")) == 1);
    ohm_fact_store_transaction_push(fs);
    ohm_fact_store_transaction_push(fs);
    ohm_fact_store_remove(fs, fact);
    ohm_fact_store_transaction_pop(fs, FALSE);
    ohm_fact_store_transaction_pop(fs, FALSE);
    fail_unless(g_slist_length(ohm_fact_store_get_facts_by_name(fs, "org.test.match")) == 0);
    (fs == NULL ? NULL : (fs = (g_object_unref(fs), NULL)));
    (fact == NULL ? NULL : (fact = (g_object_unref(fact), NULL)));
}
END_TEST


START_TEST (test_fact_store_transaction_push_and_cancel)
{
    OhmFactStore* fs;
    OhmFactStoreView* v;
    OhmFactStoreView* v2;
    OhmPattern* _tmp0;
    OhmPattern* _tmp1;
    OhmFact* fact;
    GValue* val;
    fs = ohm_fact_store_new();
    v = ohm_fact_store_new_view(fs, NULL);
    v2 = ohm_fact_store_new_view(fs, NULL);
    _tmp0 = NULL;
    ohm_fact_store_view_add(v, OHM_STRUCTURE((_tmp0 = ohm_pattern_new("org.test.match"))));
    (_tmp0 == NULL ? NULL : (_tmp0 = (g_object_unref(_tmp0), NULL)));
    _tmp1 = NULL;
    ohm_fact_store_view_add(v2, OHM_STRUCTURE((_tmp1 = ohm_pattern_new("org.freedesktop.hello"))));
    (_tmp1 == NULL ? NULL : (_tmp1 = (g_object_unref(_tmp1), NULL)));
    /* insertion*/
    ohm_fact_store_transaction_push(fs);
    {
        OhmFact* fact;
        fact = ohm_fact_new("org.test.match");
        ohm_fact_set(fact, "field", ohm_value_from_int(42));
        ohm_fact_store_insert(fs, fact);
        fail_unless(g_slist_length(ohm_fact_store_get_facts_by_name(fs, "org.test.match")) == 1);
        fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(v)->change_set)) == 0);
        (fact == NULL ? NULL : (fact = (g_object_unref(fact), NULL)));
    }
    ohm_fact_store_transaction_pop(fs, TRUE);
    fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(v)->change_set)) == 0);
    /* and from the fact store*/
    fail_unless(g_slist_length(ohm_fact_store_get_facts_by_name(fs, "org.test.match")) == 0);
    /* retract/remove*/
    fact = ohm_fact_new("org.test.match");
    ohm_fact_set(fact, "field", ohm_value_from_int(42));
    ohm_fact_store_insert(fs, fact);
    fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(v)->change_set)) == 1);
    ohm_fact_store_transaction_push(fs);
    {
        ohm_fact_store_remove(fs, fact);
        fail_unless(g_slist_length(ohm_fact_store_get_facts_by_name(fs, "org.test.match")) == 0);
        fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(v)->change_set)) == 1);
    }
    ohm_fact_store_transaction_pop(fs, TRUE);
    /* pop should remove from the view change_set*/
    fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(v)->change_set)) == 1);
    /* and reinsert in the fact store*/
    fail_unless(g_slist_length(ohm_fact_store_get_facts_by_name(fs, "org.test.match")) == 1);
    /* update*/
    ohm_fact_store_transaction_push(fs);
    {
        GValue* val;
        val = ((GValue*) ohm_fact_get(fact, "field"));
        fail_unless(g_value_get_int(val) == 42);
        ohm_fact_set(fact, "field", ohm_value_from_int(43));
        val = ((GValue*) ohm_fact_get(fact, "field"));
        fail_unless(g_value_get_int(val) == 43);
        fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(v)->change_set)) == 1);
    }
    ohm_fact_store_transaction_pop(fs, TRUE);
    /* pop should remove from the view change_set*/
    fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(v)->change_set)) == 1);
    val = ((GValue*) ohm_fact_get(fact, "field"));
    fail_unless(g_value_get_int(val) == 42);
    (fs == NULL ? NULL : (fs = (g_object_unref(fs), NULL)));
    (v == NULL ? NULL : (v = (g_object_unref(v), NULL)));
    (v2 == NULL ? NULL : (v2 = (g_object_unref(v2), NULL)));
    (fact == NULL ? NULL : (fact = (g_object_unref(fact), NULL)));
}
END_TEST


START_TEST (test_fact_store_transaction_push_and_commit)
{
    OhmFactStore* fs;
    OhmFactStoreView* v;
    OhmFactStoreView* v2;
    OhmFactStoreView* tpv;
    OhmPattern* _tmp0;
    OhmPattern* _tmp1;
    OhmFact* fact;
    GValue* val;
    fs = ohm_fact_store_new();
    v = ohm_fact_store_new_view(fs, NULL);
    v2 = ohm_fact_store_new_view(fs, NULL);
    tpv = ohm_fact_store_new_transparent_view(fs, NULL);
    _tmp0 = NULL;
    ohm_fact_store_view_add(v, OHM_STRUCTURE((_tmp0 = ohm_pattern_new("org.test.match"))));
    (_tmp0 == NULL ? NULL : (_tmp0 = (g_object_unref(_tmp0), NULL)));
    _tmp1 = NULL;
    ohm_fact_store_view_add(v2, OHM_STRUCTURE((_tmp1 = ohm_pattern_new("org.freedesktop.hello"))));
    (_tmp1 == NULL ? NULL : (_tmp1 = (g_object_unref(_tmp1), NULL)));
    _tmp1 = NULL;
    ohm_fact_store_view_add(tpv, OHM_STRUCTURE((_tmp1 = ohm_pattern_new("org.test.match"))));
    (_tmp1 == NULL ? NULL : (_tmp1 = (g_object_unref(_tmp1), NULL)));
    _tmp1 = NULL;
    /* insertion*/
    ohm_fact_store_transaction_push(fs);
    {
        fact = ohm_fact_new("org.test.match");
        ohm_fact_set(fact, "field", ohm_value_from_int(42));
        ohm_fact_store_insert(fs, fact);
        fail_unless(g_slist_length(ohm_fact_store_get_facts_by_name(fs, "org.test.match")) == 1);
        fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(v)->change_set)) == 0);
        fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(tpv)->change_set)) == 1);
    }
    ohm_fact_store_transaction_pop(fs, FALSE);
    fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(v)->change_set)) == 1);
    /* and from the fact store*/
    fail_unless(g_slist_length(ohm_fact_store_get_facts_by_name(fs, "org.test.match")) == 1);
    ohm_fact_set(fact, "field", ohm_value_from_int(43));
    fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(v)->change_set)) == 2);
    fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(tpv)->change_set)) == 2);
    ohm_fact_store_transaction_push(fs);
    {
        ohm_fact_store_remove(fs, fact);
        fail_unless(g_slist_length(ohm_fact_store_get_facts_by_name(fs, "org.test.match")) == 0);
        fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(v)->change_set)) == 2);
        fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(tpv)->change_set)) == 3);
    }
    ohm_fact_store_transaction_pop(fs, FALSE);
    fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(v)->change_set)) == 3);
    fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(tpv)->change_set)) == 3);
    fail_unless(g_slist_length(ohm_fact_store_get_facts_by_name(fs, "org.test.match")) == 0);
    /* update*/
    fact = ohm_fact_new("org.test.match");
    ohm_fact_set(fact, "field", ohm_value_from_int(41));
    ohm_fact_store_insert(fs, fact);
    fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(v)->change_set)) == 4);
    fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(tpv)->change_set)) == 4);
    ohm_fact_store_transaction_push(fs);
    {
        GValue* val;
        val = ((GValue*) ohm_fact_get(fact, "field"));
        fail_unless(g_value_get_int(val) == 41);
        ohm_fact_set(fact, "field", ohm_value_from_int(42));
        val = ((GValue*) ohm_fact_get(fact, "field"));
        fail_unless(g_value_get_int(val) == 42);
        fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(v)->change_set)) == 4);
        fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(tpv)->change_set)) == 5);
    }
    ohm_fact_store_transaction_pop(fs, FALSE);
    fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(v)->change_set)) == 5);
    fail_unless(g_slist_length(ohm_fact_store_change_set_get_matches(OHM_FACT_STORE_SIMPLE_VIEW(tpv)->change_set)) == 5);
    val = ((GValue*) ohm_fact_get(fact, "field"));
    fail_unless(g_value_get_int(val) == 42);
    (fs == NULL ? NULL : (fs = (g_object_unref(fs), NULL)));
    (v == NULL ? NULL : (v = (g_object_unref(v), NULL)));
    (v2 == NULL ? NULL : (v2 = (g_object_unref(v2), NULL)));
    (fact == NULL ? NULL : (fact = (g_object_unref(fact), NULL)));
}
END_TEST


START_TEST (test_fact_store_transaction_free)
{
    test_fact_store_transaction_push_and_cancel(_i);
}
END_TEST





TCase *
factstore_case (int desired_step_id)
{
    g_type_init();

	int step_id = 1;
	#define PREPARE_TEST(tc, fun) if (desired_step_id == 0 || step_id++ == desired_step_id)  tcase_add_test (tc, fun);
  #define PREPARE_LOOP_TEST(tc, fun, reps) \
      if (desired_step_id == 0 || step_id++ == desired_step_id)  tcase_add_loop_test (tc, fun, 0, reps);

	TCase *tc_factstore = tcase_create ("factstore");
    tcase_set_timeout(tc_factstore, 60);
    PREPARE_TEST (tc_factstore, test_fact_structure_new);
    PREPARE_TEST (tc_factstore, test_fact_structure_name);
    PREPARE_TEST (tc_factstore, test_fact_structure_set_get);
    PREPARE_LOOP_TEST (tc_factstore, test_fact_structure_free, 20000);
    PREPARE_TEST (tc_factstore, test_fact_structure_to_string);
    PREPARE_TEST (tc_factstore, test_fact_fact_new);
    PREPARE_TEST (tc_factstore, test_fact_fact_set_get);
    PREPARE_TEST (tc_factstore, test_fact_pattern_new);
    PREPARE_TEST (tc_factstore, test_fact_pattern_new_for_fact);
    PREPARE_TEST (tc_factstore, test_fact_pattern_set_get);
    PREPARE_LOOP_TEST (tc_factstore, test_fact_pattern_free, 1000);
    PREPARE_TEST (tc_factstore, test_fact_pattern_match);
    PREPARE_TEST (tc_factstore, test_fact_pattern_match_instance);
    PREPARE_TEST (tc_factstore, test_fact_pattern_match_fields);
    PREPARE_LOOP_TEST (tc_factstore, test_fact_pattern_match_free, 1000);
    PREPARE_TEST (tc_factstore, test_fact_store_new);
    PREPARE_TEST (tc_factstore, test_fact_store_insert);
    PREPARE_TEST (tc_factstore, test_fact_store_to_string);
    PREPARE_TEST (tc_factstore, test_fact_store_insert_remove);
    PREPARE_LOOP_TEST (tc_factstore, test_fact_store_free, 1000);
    PREPARE_TEST (tc_factstore, test_fact_store_view_new);
    PREPARE_TEST (tc_factstore, test_fact_store_view_two);
    PREPARE_LOOP_TEST (tc_factstore, test_fact_store_view_free, 1000);
    PREPARE_TEST (tc_factstore, test_fact_store_transaction_push_pop);
    PREPARE_TEST (tc_factstore, test_fact_store_transaction_push_and_watch);
    PREPARE_TEST (tc_factstore, test_fact_store_transaction_push_and_cancel);
    PREPARE_LOOP_TEST (tc_factstore, test_fact_store_transaction_free, 1000);
    PREPARE_TEST (tc_factstore, test_fact_store_pattern_delete);
    PREPARE_TEST (tc_factstore, test_fact_store_transaction_push_and_commit);

    return tc_factstore;
}




int
main(int argc, char* argv[]) {
	Suite *s = suite_create ("factstore");

	int step_id = 0;
	if (argc == 2) {
		step_id = strtol(argv[1], NULL, 10);
	}
	suite_add_tcase (s, factstore_case(step_id));

	SRunner *sr = srunner_create (s);
	srunner_set_fork_status (sr, CK_NOFORK);
	srunner_run_all (sr, CK_NORMAL);
	int number_failed = srunner_ntests_failed (sr);
	srunner_free (sr);

	return number_failed;
}



static int _vala_strcmp0(const char * str1, const char * str2)
{
    if (str1 == NULL) {
        return -(str1 != str2);
    }
    if (str2 == NULL) {
        return (str1 != str2);
    }
    return strcmp(str1, str2);
}




