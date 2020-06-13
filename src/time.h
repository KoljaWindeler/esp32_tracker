#define SECS_PER_MIN  ((uint32_t)(60UL))
#define SECS_PER_HOUR ((uint32_t)(3600UL))
#define SECS_PER_DAY  ((uint32_t)(SECS_PER_HOUR * 24UL))

#define SECS_PER_WEEK ((uint32_t)(SECS_PER_DAY * DAYS_PER_WEEK))
#define SECS_PER_YEAR ((uint32_t)(SECS_PER_DAY * 365UL)) // TODO: ought to handle leap years
#define SECS_YR_2000  ((uint32_t)(946684800UL)) // the time at the start of y2k
#define LEAP_YEAR(Y)     ( ((1970+(Y))>0) && !((1970+(Y))%4) && ( ((1970+(Y))%100) || !((1970+(Y))%400) ) )

