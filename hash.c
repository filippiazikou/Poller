#include "poller.h"


vote *hash_table[HASH_CAPACITY];

int hash_init() {
	int i;
	
	for (i = 0 ; i <  HASH_CAPACITY ; i++)
		hash_table[i] = NULL;
	return HASH_OK;
}


int hash_function(char* value)
{
	int length, i, k = 0, result = 0;
	char *new_string;
	float new_float;

	length = strlen( value );
	new_string = malloc( length + 1 );
	strcpy(new_string, value);

	for(i=0; i<length; i++) {
		k += new_string[i];	
	}

	result = k % HASH_CAPACITY;
	
	free( new_string );

   return result;
}

int hash_insert(vote *v) {
	int hashNum;
	vote *tempv;
	
	
	hashNum = hash_function(v->username);
	
	/*First vote for this place in array*/
	if (hash_table[hashNum] == NULL) {
		hash_table[hashNum] = malloc(sizeof(vote));
		if (hash_table[hashNum] == NULL) {
			fprintf(stderr, "Error while allocating memory in hash_insert\n");
			return HASH_ERROR;
		}
		memcpy(hash_table[hashNum] , v, sizeof(vote));
		hash_table[hashNum] -> next == NULL;
	}
	
	else {
		tempv = hash_table[hashNum];
		while (tempv->next != NULL)
			tempv = tempv->next;
		tempv->next = malloc(sizeof(vote));
		if (tempv->next == NULL) {
			fprintf(stderr, "Error while allocating memory in hash_insert!\n");
			return HASH_ERROR;
		}
		tempv = tempv->next;
		memcpy(tempv , v, sizeof(vote));
		tempv -> next == NULL;
		
	}
	
	return HASH_OK;
}

int hash_scan (char *username) {
	int hashNum;
	vote *tempv;
	
	hashNum = hash_function(username);
	if (hash_table[hashNum] == NULL) 
		return NOT_FOUND;
	
	tempv = hash_table[hashNum];
	
	for (;;) {
		if (strcmp (tempv->username, username) == 0)
			return FOUND;
		if (tempv->next == NULL)
			break;
		else
			tempv = tempv->next;

	}
	
	return NOT_FOUND;
}


void hash_free() {
	vote *last;
	vote *init;
	int i;
	
	for ( i = 0 ; i < HASH_CAPACITY ; i++ ) {
		if (hash_table[i] != NULL) {
			for (;;) {
				init = hash_table[i];
				last = init;
				while (last -> next != NULL) {
					last = last->next;
				}
				free(last);
				last = NULL;
				if (last == init)
					break;
			}
			
		}
	}
}