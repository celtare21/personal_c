#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "calendar.h"

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#else
#include <unistd.h>
#endif

#if defined(_WIN32) || defined(_WIN64)
#define clear() system("cls")
#else
#define clear() system("clear")
#endif

#define JAN 31
#define FEB_L 29
#define MAR 31
#define APR 30
#define MAY 31
#define JUN 30
#define JUL 31
#define AUG 31
#define SEP 30
#define OCT 31
#define NOV 30
#define DEC 31

#define MONTHS 12
#define LISTS 20

#define ALL 1
#define MONTH 2
#define DAY 3

#define SIZE 128

struct List {
    int day;
    char task[SIZE];
    bool set;
};

struct Months {
    struct List* list;
    int days;
};

static struct Months* init(void);
static inline void addNewEntry(struct Months* month);
static void readAllEntries(struct Months* month, unsigned int flags, unsigned int month_number, unsigned int day_number);
static inline void readEntry(struct Months* month);
static inline void removeEntry(struct Months* month);
static void writeFile(struct Months* month);
static void readFile(struct Months* month);
static void freeMem(struct Months* month);
static inline int getCurrentDate(bool date);
static void ClearInputBuffer(void);

void Calendar_launch(void)
{
    struct Months* month;
    char c;

    month = init();

    readFile(month);

    while (true) {
        printf("Current month: %d\n", getCurrentDate(true));
        printf("Current day: %d\n\n", getCurrentDate(false));
        printf("(A)dd entry, (R)emove entry, (S)how entry, (E)xit\n");
        (void)scanf(" %c", &c);
        if (c == 'A' || c == 'a')
            addNewEntry(month);
        if (c == 'R' || c == 'r')
            removeEntry(month);
        if (c == 'S' || c == 's')
            readEntry(month);
        if (c == 'E' || c == 'e')
            break;
    }

    writeFile(month);

    freeMem(month);
}

static struct Months* init(void)
{
    struct Months* month;
    int i, j;

    clear();

    month = (struct Months*)malloc(MONTHS * sizeof(struct Months));
    if (!(month))
        return NULL;

    month[0].days = JAN;
    month[1].days = FEB_L;
    month[2].days = MAR;
    month[3].days = APR;
    month[4].days = MAY;
    month[5].days = JUN;
    month[6].days = JUL;
    month[7].days = AUG;
    month[8].days = SEP;
    month[9].days = OCT;
    month[10].days = NOV;
    month[11].days = DEC;

    for (i = 0; i < MONTHS; i++) {
        month[i].list = (struct List*)malloc(LISTS * sizeof(struct List));
        if (!month[i].list)
            return NULL;
        for (j = 0; j < LISTS; j++)
            month[i].list[j].set = false;
        for (j = 0; j < LISTS; j++)
            month[i].list[j].day = -1;
    }

    return month;
}

static inline void addNewEntry(struct Months* month)
{
    int i, day;
    static int j = 0, last = -1;
    char str[SIZE], c[13];

    clear();

    printf("Current month: %d\n", getCurrentDate(true));
    printf("Current day: %d\n\n", getCurrentDate(false));

retry_month:
    printf("Which month?\n");
    printf("Say 'C' for current month\n");
    (void)scanf("%s", &c);
    c[strlen(c)] = '\0';
    i = atoi(c);
    if (!i)
        i = getCurrentDate(true);
    else if (i < 1 || i > 12) {
        printf("Enter a valid month\n");
        goto retry_month;
    }

retry_day:
    printf("Which day?\n");
    printf("Say 'C' for current day\n");
    (void)scanf("%s", &c);
    day = atoi(c);
    if (!day)
        day = getCurrentDate(false);
    else if (day < 1 || day > month[i - 1].days) {
        printf("Enter a valid day\n");
        goto retry_day;
    }

    if (last != i)
        j = 0;

    ClearInputBuffer();
    printf("What task?\n");
    fgets(str, sizeof(str), stdin);
    strcpy(month[i - 1].list[j].task, str);

    month[i - 1].list[j].day = day;
    month[i - 1].list[j].set = true;

    ++j;
    last = i;
    clear();
}

static void readAllEntries(struct Months* month, unsigned int flags, unsigned int month_number, unsigned int day_number)
{
    int i, j;

    clear();

    if (flags == ALL)
        for (i = 0; i < MONTHS; i++)
            for (j = 0; j < LISTS; j++)
                if (month[i].list[j].set) {
                    printf("On month %d, on day %d you have: \n", i + 1, month[i].list[j].day);
                    printf("%d. %s\n", j + 1, month[i].list[j].task);
                }
    if (flags == MONTH)
        for (j = 0; j < LISTS; j++)
            if (month[month_number - 1].list[j].set) {
                printf("On month %d, on day %d you have: \n", month_number, month[month_number - 1].list[j].day);
                printf("%d. %s\n", j + 1, month[month_number - 1].list[j].task);
            }
    if (flags == DAY)
        for (j = 0; j < LISTS; j++)
            if (month[month_number - 1].list[j].set && month[month_number - 1].list[j].day == day_number) {
                printf("On month %d, on day %d you have: \n", month_number, month[month_number - 1].list[j].day);
                printf("%d. %s\n", j + 1, month[month_number - 1].list[j].task);
            }
}

static inline void readEntry(struct Months* month)
{
    unsigned int flags = 0, month_number = 0, day_number = 0;
    int val;
    char c, x[13];

    clear();

    printf("Current month: %d\n", getCurrentDate(true));
    printf("Current day: %d\n\n", getCurrentDate(false));

    printf("(A)ll entries, Entries on a specific (M)onth, Entries on a specific (D)ay\n");
    (void)scanf(" %c", &c);

    if (c == 'A' || c == 'a')
        flags = ALL;
    if (c == 'M' || c == 'm') {
        flags = MONTH;
        printf("Which month?\n");
        printf("Say 'C' for current month\n");
        (void)scanf("%s", &x);
        x[strlen(x)] = '\0';
        val = atoi(x);
        (!val) ? (month_number = getCurrentDate(true)) : (month_number = val);
    }
    if (c == 'D' || c == 'd') {
        flags = DAY;
        printf("Which month?\n");
        printf("Say 'C' for current month\n");
        (void)scanf("%s", &x);
        x[strlen(x)] = '\0';
        val = atoi(x);
        (!val) ? (month_number = getCurrentDate(true)) : (month_number = val);
        printf("Which day?\n");
        printf("Say 'C' for current day\n");
        (void)scanf("%s", &x);
        x[strlen(x)] = '\0';
        val = atoi(x);
        (!val) ? (day_number = getCurrentDate(false)) : (day_number = val);
    }

    readAllEntries(month, flags, month_number, day_number);
}

static inline void removeEntry(struct Months* month)
{
    unsigned int month_number, entry_number;
    int val;
    char c[13];

    readAllEntries(month, ALL, 0, 0);

    printf("Current month: %d\n", getCurrentDate(true));
    printf("Current day: %d\n\n", getCurrentDate(false));

    printf("Which month?\n");
    printf("Say 'C' for current month\n");
    (void)scanf("%s", &c);
    c[strlen(c)] = '\0';
    val = atoi(c);
    (!val) ? (month_number = getCurrentDate(true)) : (month_number = val);
    printf("Which entry?\n");
    (void)scanf("%d", &entry_number);

    if (month[month_number - 1].list[entry_number - 1].set) {
        memset(month[month_number - 1].list[entry_number - 1].task, 0, SIZE);
        month[month_number - 1].list[entry_number - 1].set = false;
        month[month_number - 1].list[entry_number - 1].day = -1;
    }
}

static void writeFile(struct Months* month)
{
    int i, j;
    FILE* file;

    file = fopen("to-do.txt", "w");
    if (!file)
        return;

    for (i = 0; i < MONTHS; i++)
        for (j = 0; j < LISTS; j++)
            if (month[i].list[j].set)
                fprintf(file, "%d %d %d %s\n", i, j, month[i].list[j].day, month[i].list[j].task);

    fclose(file);
}

static void readFile(struct Months* month)
{
    int i, j, day;
    char str[SIZE];
    FILE* file;

    file = fopen("to-do.txt", "r");
    if (!file)
        return;

    while (fscanf(file, "%d %d %d", &i, &j, &day) != EOF) {
        fgets(str, SIZE, file);
        month[i].list[j].day = day;
        strcpy(month[i].list[j].task, str);
        month[i].list[j].set = true;
    }

    fclose(file);
}

static void freeMem(struct Months* month)
{
    int i;

    for (i = 0; i < MONTHS; i++)
        free(month[i].list);
    free(month);
}

static inline int getCurrentDate(bool date)
{
    time_t t = time(NULL);
    struct tm current_time = *localtime(&t);

    return date ? current_time.tm_mon + 1 : current_time.tm_mday;
}

static void ClearInputBuffer(void)
{
    char c;

    while ((c = getchar()) != '\n' && c != EOF);
}