
#ifndef __libohm_marshal_MARSHAL_H__
#define __libohm_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* NONE:STRING,INT (libohm-marshal.list:1) */
extern void libohm_marshal_VOID__STRING_INT (GClosure     *closure,
                                             GValue       *return_value,
                                             guint         n_param_values,
                                             const GValue *param_values,
                                             gpointer      invocation_hint,
                                             gpointer      marshal_data);
#define libohm_marshal_NONE__STRING_INT	libohm_marshal_VOID__STRING_INT

G_END_DECLS

#endif /* __libohm_marshal_MARSHAL_H__ */

