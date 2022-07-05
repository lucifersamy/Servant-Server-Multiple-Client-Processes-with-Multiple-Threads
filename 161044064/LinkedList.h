#ifndef LINKED_LIST
#define LINKED_LIST

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#define SIZE 200

/*Elements in file lines*/
struct Transaction{
    struct Transaction* next;
    char transactionType[SIZE], transactionStreet[SIZE];
    int transactionId, transactionSurface, transactionPrice;
};
/*Dates from file names*/
struct Date{
    struct Transaction* head;
    struct Date* next;
    int day, month, year;
};
/*City names from dir names*/
struct City{
    struct Date* head;
    struct City* next;
    char cityName[SIZE];
};
/*All data*/
struct Country{
    struct City* head;
};


typedef struct Transaction Transaction;
typedef struct Date Date;
typedef struct City City;
typedef struct Country Country;


Transaction* transactionInit(char* newContent);
City* cityInit(char* newCity);
Date* dateInit(char* newDate);

int insertTransactionToDate(Date* date, Transaction* newTransaction);
int insertDateToCity(City* city, Date* newDate);
int insertCityToCountry(Country* country, City* newCity);

void transactionFree(Transaction* transaction);
void dateFree(Date* file);
void cityFree(City* city);
void countryFree(Country* country);

int searchInCountry(Country* country, char* cityName, char* d1, char* d2, char* type);
int isInRange(Date* date, char* d1, char* d2);

#endif