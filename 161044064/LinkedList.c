#include "LinkedList.h"


Transaction* transactionInit(char* newContent)
{
    char newTransactionType[SIZE], newTransactionStreet[SIZE] ;
    int newId, newTransactionSurface, newTransactionPrice;
    bzero(newTransactionType,sizeof(newTransactionType));
    bzero(newTransactionStreet,sizeof(newTransactionStreet));
    
    sscanf(newContent, "%d %s %s %d %d\n", &newId, newTransactionType, newTransactionStreet, &newTransactionSurface, &newTransactionPrice);


    Transaction* transaction = calloc(1, sizeof (Transaction));

    transaction->transactionId = newId;
    transaction->transactionSurface = newTransactionSurface;
    transaction->transactionPrice = newTransactionPrice;

    strcpy(transaction->transactionType, newTransactionType);
    strcpy(transaction->transactionStreet, newTransactionStreet);

    return transaction;
}

City* cityInit(char* newCity)
{
    City* city = calloc(1, sizeof (City));
    strcpy(city->cityName, newCity);
    return city;
}

Date* dateInit(char* newDate)
{
    Date* date = calloc(1, sizeof (Date));

    int newDay=0,newMonth=0,newYear=0;

    sscanf(newDate, "%d-%d-%d", &newDay, &newMonth, &newYear);
    
    
    date->day = newDay;
    date->month = newMonth;
    date->year = newYear;

    return date;    
}

int insertTransactionToDate(Date* date, Transaction* newTransaction)
{
    if(date == NULL){
        fprintf(stderr, "Date is NULL\n");
        exit(EXIT_FAILURE);
    }

    if(newTransaction == NULL){ 
        fprintf(stderr, "Transaction is NULL\n");
        exit(EXIT_FAILURE);
    }
    
    if(date->head == NULL)
    {
        date->head = newTransaction;
        return 0;
    }

    Transaction* temp = NULL;
    temp = date->head;
    while(temp->next != NULL){
        temp = temp->next;
    }

    temp->next = newTransaction;
    return 0;
}

int insertDateToCity(City* city, Date* newDate)
{
    if(city == NULL){ 
        fprintf(stderr, "City is NULL\n");
        exit(EXIT_FAILURE);
    }
    if(newDate == NULL){
        fprintf(stderr, "Date is NULL\n");
        exit(EXIT_FAILURE);
    }

    if(city->head == NULL)
    {
        city->head = newDate;
        return 0;
    }

    Date* temp = NULL;

    temp = city->head;
    while(temp->next != NULL){
        temp = temp->next;
    }

    temp->next = newDate;
    return 0;
}


int insertCityToCountry(Country* country, City* newCity)
{
    if(country == NULL){ 
        fprintf(stderr, "Country is NULL\n");
        exit(EXIT_FAILURE);
    }
    if(newCity == NULL){
        fprintf(stderr, "Country is NULL\n");
        exit(EXIT_FAILURE);
    }
    
    if(country->head == NULL)
    {
        country->head = newCity;
        return 0;
    }

    City* temp = NULL;
    temp = country->head;

    while(temp->next != NULL){
        temp = temp->next;
    }

    temp->next = newCity;
    return 0;
}



void transactionFree(Transaction* transaction)
{
    if(transaction == NULL)
        return;

    transactionFree(transaction->next);
    free(transaction);
}



void dateFree(Date* file)
{
    if(file == NULL)
        return;

    transactionFree(file->head);
    dateFree(file->next);
    free(file);
}
void cityFree(City* city)
{
    if(city == NULL)
        return;

    dateFree(city->head);
    cityFree(city->next);
    free(city);
}

void countryFree(Country* country)
{
    if(country == NULL)
        return;

    cityFree(country->head);
    free(country);
}


int searchInCountry(Country* country, char* cityName, char* d1, char* d2, char* type){

    int totalTransactionNum=0;

    if(cityName == NULL){
        for(City* temp1 = country->head ; temp1 != NULL ; temp1 = temp1->next){
            for(Date* temp2 = temp1->head ; temp2 != NULL ; temp2 = temp2->next){
                if(isInRange(temp2,d1,d2)){
                    for(Transaction* temp3 = temp2->head ; temp3 != NULL ; temp3 = temp3->next){
                        if(strcmp(temp3->transactionType, type) == 0){
                            totalTransactionNum++;
                        }
                    }
                }
            }
        }
    }
    else{
        for(City* temp1 = country->head ; temp1 != NULL ; temp1 = temp1->next){
            if(strcmp(cityName, temp1->cityName) == 0){
                for(Date* temp2 = temp1->head ; temp2 != NULL ; temp2 = temp2->next){
                    if(isInRange(temp2,d1,d2)){
                        for(Transaction* temp3 = temp2->head ; temp3 != NULL ; temp3 = temp3->next){
                            if(strcmp(temp3->transactionType, type) == 0){
                                totalTransactionNum++;
                            }
                        }
                    }
                }
                break;
            }
        }
    }

    return totalTransactionNum;
}

int isInRange(Date* date, char* d1, char* d2){
    int startDay=0,startMonth=0,startYear=0;
    int num;
    num = sscanf(d1, "%d-%d-%d", &startDay, &startMonth, &startYear);
    if(num!=3){
        return -1;
    }
    int endDay=0,endMonth=0,endYear=0;
    num = sscanf(d2, "%d-%d-%d", &endDay, &endMonth, &endYear);
    if(num!=3){
        return -1;
    }

    if((date->year < startYear) || date->year > endYear){
        return 0;
    }
    if(date->year == startYear){
        if(date->month < startMonth){
            return 0;
        }
        else if(date->month == startMonth){
            if(date->day < startDay){
                return 0;
            }
        }
    }

    if(date->year == endYear){
        if(date->month > endMonth){
            return 0;
        }
        else if(date->month == endMonth){
            if(date->day > endDay){
                return 0;
            }
        }
    }
    

    return 1;
}