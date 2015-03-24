char *get_hint          args ( ( int level ) );
#define HINT_FILE       SYSTEM_DIR "hints.txt"   /* For Hints */
typedef struct  hint_data               HINT_DATA;
struct hint_data
{
        HINT_DATA       *next;
        HINT_DATA       *prev;
        char            *text;
        int             low;
        int             high;
};
extern          HINT_DATA       * hint;
extern          HINT_DATA       *first_hint;
extern          HINT_DATA       *last_hint;

#define HINT_UPDATEFREQ         1

