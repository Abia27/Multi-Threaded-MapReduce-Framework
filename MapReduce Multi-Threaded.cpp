#include <iostream>
#include <pthread.h>
#include <cstring>
#include <cstdlib>
#include <string>

using namespace std;

struct KeyValuePair
{
    char word[50];
    int counts[100];
    int countIndex;
};

// Array to store word counts
KeyValuePair keyValuePairs[100];

// Counter for key-value pairs
int pairCount = 0;

pthread_mutex_t mutexLock;

// Map Function
void *mapFunction(void *arg)
{
    string *text = (string *)arg;

    // If the string is empty exit
    if (text->empty()) {
        pthread_exit(NULL);
    }

    char temp[text->size() + 1];
    strcpy(temp, text->c_str());
    char *word = strtok(temp, " ");

    while (word != NULL)
    {
        // Skip empty words
        if (strlen(word) == 0)
        {
            word = strtok(NULL, " ");
            continue;
        }

        pthread_mutex_lock(&mutexLock);

        // Check if the word is already in the keyValuePairs array
        bool found = false;
        
        for (int i = 0; i < pairCount; i++)
        {
            if (strcmp(keyValuePairs[i].word, word) == 0)
            {
                keyValuePairs[i].counts[keyValuePairs[i].countIndex++] = 1;
                found = true;
                break;
            }
        }

        // If the word isn't found, create a new entry for it
        if (!found)
        {
            strcpy(keyValuePairs[pairCount].word, word);
            keyValuePairs[pairCount].counts[0] = 1;
            keyValuePairs[pairCount].countIndex = 1;
            pairCount++;
        }

        pthread_mutex_unlock(&mutexLock);

        word = strtok(NULL, " ");
    }

    pthread_exit(NULL);
}


// Reduce Function
void *reduceFunction(void *arg)
{
    int index = *(int *)arg;

    pthread_mutex_lock(&mutexLock);
   
    int totalCount = 0;
    
    for (int i = 0; i < keyValuePairs[index].countIndex; i++)
    {
        totalCount += keyValuePairs[index].counts[i];
    }
    
    cout << "(" << keyValuePairs[index].word << ", " << totalCount << ")" << endl;
   
    pthread_mutex_unlock(&mutexLock);

    pthread_exit(NULL);
}

// Shuffle function
void showShuffleResults()
{
    cout << "\nShuffle Phase Results:" << endl;
    for (int i = 0; i < pairCount; i++)
    {
        cout << "(\"" << keyValuePairs[i].word << "\", [";
        for (int j = 0; j < keyValuePairs[i].countIndex; j++)
        {
            cout << keyValuePairs[i].counts[j];
            if (j < keyValuePairs[i].countIndex - 1)
            {
                cout << ", ";
            }
        }
        
        cout << "])";
        if (i < pairCount - 1)
        {
            cout << ", ";
        }
    }
    cout << endl;
}

int main()
{
    pthread_mutex_init(&mutexLock, NULL);

    // Prompt user to enter number of strings
    int numOfStrings;
    cout << "Enter the number of strings: ";
    cin >> numOfStrings;
    cin.ignore();

    if (numOfStrings > 50)
    {
        cout << "You can only enter a maximum of 50 strings." << endl;
        return 1;
    }

    // Storing strings in textChunks
    string textChunks[numOfStrings];
    
    for (int i = 0; i < numOfStrings; i++)
    {
        cout << "Enter string " << i + 1 << ": ";
        getline(cin, textChunks[i]);

        // If the string is empty, ask again for valid input
        if (textChunks[i].empty())
        {
            cout << "You entered an empty string. Please enter a non-empty string." << endl;
            i--;
        }
    }


    // Map Thread
    pthread_t mapThreads[100];

    for (int i = 0; i < numOfStrings; i++)
    {
        pthread_create(&mapThreads[i], NULL, mapFunction, (void *)&textChunks[i]);
    }

    for (int i = 0; i < numOfStrings; i++)
    {
        pthread_join(mapThreads[i], NULL);
    }

    // Display Shuffle Result 
    showShuffleResults();

    cout << "\nFinal Output:" << endl;
    
    // Reduce Thread
    pthread_t reduceThreads[100];
    int reduceArgs[pairCount];

    for (int i = 0; i < pairCount; i++)
    {
        reduceArgs[i] = i;
        pthread_create(&reduceThreads[i], NULL, reduceFunction, &reduceArgs[i]);
    }

    for (int i = 0; i < pairCount; i++)
    {
        pthread_join(reduceThreads[i], NULL);
    }

    pthread_mutex_destroy(&mutexLock);

    return 0;
}
