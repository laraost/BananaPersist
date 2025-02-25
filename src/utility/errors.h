// massert based on https://stackoverflow.com/a/37264642

#ifndef NDEBUG
    #define massert(Expression, Message) \
        massert_func(#Expression, Expression, __FILE__, __LINE__, Message)
#else
    #define massert(Expression, Message) do {} while(false)
#endif

void massert_func(const char* expression_string, bool expression, const char* filename, int linenumber, const char* message);