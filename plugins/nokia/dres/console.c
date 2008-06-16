
#define MAX_ARGS 32                        /* to dres as local variables */


static int parse_dres_args(char *input, char **args, int narg);

static command_t *find_command(char *name);

static void command_help   (int id, char *input);
static void command_dump   (int id, char *input);
static void command_set    (int id, char *input);
static void command_resolve(int id, char *input);
static void command_prolog (int id, char *input);
static void command_bye    (int id, char *input);
static void command_grab   (int id, char *input);
static void command_release(int id, char *input);
static void command_debug  (int id, char *input);

#define COMMAND(c, a, d) {                                      \
    name:       #c,                                             \
    args:        a,                                             \
    description: d,                                             \
    handler:     command_##c,                                   \
}
#define END { name: NULL }

static command_t commands[] = {
    COMMAND(help   , NULL       , "Get help on the available commands."     ),
    COMMAND(dump   , "[var]"    , "Dump a given or all factstore variables."),
    COMMAND(set    , "var value", "Set/change a given fact store variable." ),
    COMMAND(resolve, "[goal]"   , "Run the dependency resolver for a goal." ),
    COMMAND(prolog , NULL       , "Start an interactive prolog shell."      ),
    COMMAND(bye    , NULL       , "Close the resolver terminal session."    ),
    COMMAND(grab   , NULL       , "Grab stdout and stderr to this terminal."),
    COMMAND(release, NULL       , "Release any previous grabs."             ),
    COMMAND(debug  , "list|set...", "Configure runtime debugging/tracing."  ),
    END
};

static int console;



/********************
 * console_init
 ********************/
static int
console_init(char *address)
{
    console = console_open(address,
                           console_opened, console_closed, console_input,
                           NULL, FALSE);

    return console < 0 ? EINVAL : 0;
}


/********************
 * console_exit
 ********************/
static void
console_exit(void)
{
    /*
     * XXX TODO: Currently OHM does not clean up plugins in reverse
     *           dependency order so cross-plugin calls upon cleanup
     *           are not safe. 
     */
#if 0
    if (console > 0)
        console_close(console);
#endif

    console = 0;
}



/*****************************************************************************
 *                         *** console event handlers ***                    *
 *****************************************************************************/

/********************
 * console_opened
 ********************/
static void
console_opened(int id, struct sockaddr *peer, int peerlen)
{
    printf("***** console 0x%x opened.\n", id);

    console_printf(id, "OHMng Dependency Resolver Console\n");
    console_printf(id, "Type help to get a list of available commands.\n\n");
    console_printf(id, CONSOLE_PROMPT);
}


/********************
 * console_closed
 ********************/
static void
console_closed(int id)
{
    printf("***** console 0x%x closed.\n", id);
}


/********************
 * console_input
 ********************/
static void
console_input(int id, char *input, void *data)
{
    command_t *command;
    char       name[64], *args, *s, *d;
    int        n;

    if (!input[0]) {
        console_printf(id, CONSOLE_PROMPT);
        return;
    }
    
    n = 0;
    s = input;
    d = name;
    while (*s && *s != ' ' && n < sizeof(name) - 1)
        *d++ = *s++;
    *d = '\0';

    args = s;
    while (*args == ' ' || *args == '\t')
        args++;

    if ((command = find_command(name)) != NULL)
        command->handler(id, args);
    else
        console_printf(id, "unknown console command \"%s\"\n", input);
    
    console_printf(id, CONSOLE_PROMPT);
}



/*****************************************************************************
 *                       *** console command handlers ***                    *
 *****************************************************************************/


/********************
 * command_bye
 ********************/
static void
command_bye(int id, char *input)
{
    console_close(id);
}


/********************
 * command_dump
 ********************/
static void
command_dump(int id, char *input)
{
    OhmFactStore *fs = ohm_fact_store_get_fact_store();;
    OhmFact      *fact;
    GSList       *list;

    char factname[128], fullname[128], *name, *p, *q, *dump;
    
    p = input;
    if (!*p)
        p = "all";
    while (*p) {
        while (*p == ' ' || *p == ',')
            p++;
        for (q = factname; isalnum(*p) || *p == '_' || *p == '.'; *q++ = *p++)
            ;
        *q = '\0';
        if (!strcmp(factname, "all")) {
            dump = ohm_fact_store_to_string(fs);
            console_printf(id, "fact store: %s\n", dump);
            g_free(dump);
        }
        else {
            if (factname[0] == '.') {
                sprintf(fullname, "%s%s", dres_get_prefix(dres), factname+1);
                name = fullname;
            }
            else
                name = factname;
            for (list = ohm_fact_store_get_facts_by_name(fs, name);
                 list != NULL;
                 list = g_slist_next(list)) {
                fact = (OhmFact *)list->data;
                dump = ohm_structure_to_string(OHM_STRUCTURE(fact));
                console_printf(id, "%s\n", dump ?: "");
                g_free(dump);
            }
        }
    }
}


/********************
 * command_set
 ********************/
static void
command_set(int id, char *input)
{
    set_fact(id, input);    
}


/********************
 * command_resolve
 ********************/
static void
command_resolve(int id, char *input)
{
    char *goal;
    char *args[MAX_ARGS * 2 + 1];
    int   i;

    if (!input[0]) {
        goal    = "all";
        args[0] = NULL;
    }
    else {
        goal = input;
        while (*input && *input != ' ' && *input != '\t')
            input++;
        if (*input)
            *input++ = '\0';
        if (parse_dres_args(input, args, MAX_ARGS) != 0) {
            console_printf(id, "failed to parse arguments\n");
            return;
        }
    }
    
    console_printf(id, "updating goal \"%s\"\n", goal);
    if (args[0]) {
        console_printf(id, "  with the following arguments:\n");
        for (i = 0; args[i] != NULL; i += 2)
            console_printf(id, "    %s=%s\n", args[i], args[i+1]);
    }
    
    dres_update_goal(dres, goal, args);
}


/********************
 * command_prolog
 ********************/
static void
command_prolog(int id, char *input)
{
    prolog_shell();
}


/********************
 * command_grab
 ********************/
static void
command_grab(int id, char *input)
{
    console_grab(id, 0);
    console_grab(id, 1);
    console_grab(id, 2);
}

/********************
 * command_release
 ********************/
static void
command_release(int id, char *input)
{
    console_ungrab(id, 0);
    console_ungrab(id, 1);
    console_ungrab(id, 2);
}


/********************
 * command_debug
 ********************/
static void
command_debug(int id, char *input)
{
    char buf[1024];

    if (!strcmp(input, "list") || !strcmp(input, "help")) {
        trace_list_flags(NULL, buf, sizeof(buf),
                         "  %-25.25F %-30.30d [%-3.3s]", NULL);
        console_printf(id, "The available debug flags are:\n%s\n", buf);
    }
    else if (!strcmp(input, "disable") || !strcmp(input, "off")) {
        trace_disable(NULL);
        console_printf(id, "Debugging is now turned off.\n");
    }
    else if (!strcmp(input, "enable") || !strcmp(input, "on")) {
        trace_enable(NULL);
        console_printf(id, "Debugging is now turned on.\n");
    }
    else if (!strncmp(input, "set ", 4)) {
        if (trace_parse_flags(input + 4))
            console_printf(id, "failed to parse debugging flags.\n");
        else
            console_printf(id, "Debugging configuration updated.\n");
    }
}


/********************
 * command_help
 ********************/
static void
command_help(int id, char *input)
{
    command_t *c;
    char       syntax[128];

    console_printf(id, "Available commands:\n");
    for (c = commands; c->name != NULL; c++) {
        sprintf(syntax, "%s%s%s", c->name, c->args ? " ":"", c->args ?: ""); 
        console_printf(id, "    %-30.30s %s\n", syntax, c->description);
    }
}


/********************
 * find_command
 ********************/
static command_t *
find_command(char *name)
{
    command_t *c;

    for (c = commands; c->name != NULL; c++)
        if (!strcmp(c->name, name))
            return c;
    
    return NULL;
}


/********************
 * parse_dres_args
 ********************/
static int
parse_dres_args(char *input, char **args, int narg)
{
    char *next, *var, *val, **arg;
    int   i;

    next = input;
    arg  = args;
    for (i = 0; next && *next && i < narg; i++) {
        while (*next == ' ')
            next++;
        var = next;
        val = strchr(next, ' ');

        if (!*var || val == NULL)
            break;
        
        *val++ = '\0';

        while (*val == ' ')
            val++;
        
        if (!*val)
            return EINVAL;
        
        if ((next = strchr(val, ' ')) != NULL)
            *next++ = '\0';
    
        *arg++ = var;
        *arg++ = val;
    }
    *arg = NULL;

    return 0;
}



#if 0
/********************
 * dres_goal
 ********************/
static void
dres_goal(int cid, char *cmd)
{
    char  buf[2048];
    char *goal;
    char *varname;
    char *value;
    char *args[MAX_ARGS * 2 + 1];
    int   i;
    char  dbgstr[MAX_ARGS * 32];
    char *p, *e, *sep;

    if (!cmd || !*cmd) 
        goto syntax_error;

    strncpy(buf, cmd, sizeof(buf));
    buf[sizeof(buf)-1] = '\0';

    goal = value = buf;
    i = 0;

    if ((varname = strchr(value, ' ')) != NULL) {
        *varname++ = '\0';

        while ((value = strchr(varname, '=')) != NULL) {
            *value++ = '\0';

            args[i++] = varname;
            args[i++] = value;
            
            if (i >= MAX_ARGS * 2 || (varname = strchr(value, ' ')) == NULL)
                break;
            *varname++ = '\0';
       }
    }

    args[i] = NULL;

    p  = dbgstr;
    e  = p + sizeof(dbgstr);
    p += snprintf(p, e-p, "building goal '%s' with arguments '", goal);
    for (i = 0, sep="";  args[i] != NULL;  i += 2, sep = " ") {
        p += snprintf(p, e-p, "%s%s %s", sep, args[i], args[i+1]);
    }
    snprintf(p, e-p, "%s<null>'", sep);
    DEBUG("%s", dbgstr);

    dres_update_goal(dres, goal, args);

    return;

 syntax_error:
    console_printf(cid, "invalid dres arguments \"%s\"\n", cmd);
}
#endif










/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */

