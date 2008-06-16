#ifndef __OHM_PLUGIN_DEBUG_H__
#define __OHM_PLUGIN_DEBUG_H__

#include <trace/trace.h>

#ifndef unlikely
#  define unlikely(cond) __builtin_expect(cond, 0)
#endif

#define OHM_DEBUG_PLUGIN(plugin, ...)				\
    TRACE_DECLARE_COMPONENT(plugin, #plugin, __VA_ARGS__)

#define OHM_DEBUG_FLAG(n, d, br)		\
    TRACE_FLAG_INIT((n), (d), (br))

#define OHM_DEBUG_INIT(plugin) ({					\
      int success;							\
      success = !trace_init() && !trace_add_component(NULL, &plugin);	\
      success;								\
    })

#define OHM_DEBUG(flag, format, args...) do {				\
    __trace_write(NULL, __FILE__, __LINE__, __FUNCTION__,		\
		  flag, NULL, format"\n", ## args);			\
  } while (0)

#define OHM_DEBUG_ON(f)     trace_on(NULL, f)
#define OHM_DEBUG_OFF(f)    trace_off(NULL, f)
#define OHM_DEBUG_ENABLE()  trace_enable(NULL)
#define OHM_DEBUG_DISABLE() trace_disable(NULL)


#endif /* __OHM_PLUGIN_DEBUG_H__ */
