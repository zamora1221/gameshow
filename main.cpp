#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

using namespace std;

struct Question {
    string question;
    string answers[4];
    char correctAnswer;
};

// Function to read questions from a file and store them in a vector
void read_questions(const string& questionFile, vector<Question>& questions) {
    ifstream qFile(questionFile);
    if (!qFile) {
        cout << "Failed to open question file." << endl;
        exit(1);
    }

    string line;
    while (getline(qFile, line)) {
        Question q;
        q.question = line;

        for (int i = 0; i < 4; ++i) {
            getline(qFile, line);
            q.answers[i] = line;
        }

        questions.push_back(q);

        // Skip the empty line
        getline(qFile, line);
    }

    qFile.close();
}

// Function to read correct answers from a file and store them in a vector
void read_answers(const string& answerFile, vector<char>& correctAnswers) {
    ifstream aFile(answerFile);
    if (!aFile) {
        cout << "Failed to open answer file." << endl;
        exit(1);
    }

    char answer;
    while (aFile >> answer) {
        correctAnswers.push_back(answer);
    }

    aFile.close();
}

// Function to display a question along with answer choices
void show_question(const Question& q, char hideOption = '\0') {
    cout << q.question << endl;

    for (int i = 0; i < 4; ++i) {
        char option = static_cast<char>('A' + i);
        if (option == hideOption) {
            continue; // Hide the specified option
        }
        cout << option << ") " << q.answers[i] << endl;
    }
}

// Function to get the player's answer choice
char player_try(const Question& q) {
    char response;
    cout << "Your answer (A/B/C/D): ";
    cin >> response;
    response = toupper(response);

    while (response != 'A' && response != 'B' && response != 'C' && response != 'D') {
        cout << "Invalid input. Please enter A, B, C, or D: ";
        cin >> response;
        response = toupper(response);
    }

    return response;
}

// Function to handle the game logic
void play_game(const vector<Question>& questions, const vector<char>& correctAnswers, const string& playerName) {
    int totalScore = 0;
    int points = 10; // Starting points for the first question
    int consecutiveCorrect = 0; // Count of consecutive correct answers

    for (int round = 0; round < questions.size(); ++round) {
        const Question& q = questions[round];
        char correctAnswer = correctAnswers[round];

        char playerAnswer;
        bool secondTry = false;

        while (true) {
            show_question(q, (secondTry ? playerAnswer : '\0'));
            playerAnswer = player_try(q);

            if (playerAnswer == correctAnswer) {
                if (!secondTry) {
                    totalScore += points;
                    consecutiveCorrect++;
                }
                else {
                    // If the player got it right on the second try, award 1/2 of 10 times the previous award
                    points = (points / 2);
                    totalScore += points;
                }

                cout << "Correct! You earned " << points << " points." << endl;
                break;
            }
            else if (secondTry) {
                cout << "Sorry, that's incorrect. The correct answer was " << correctAnswer << "." << endl;
                cout << "Game over! You won $" << totalScore << "." << endl;

                // Save the current player's name and score to "summary.txt" in append mode
                ofstream summaryFile("summary.txt", ios::app);
                if (!summaryFile) {
                    cerr << "Failed to open summary file." << endl;
                    exit(1);
                }
                summaryFile << playerName << " " << totalScore << endl;
                summaryFile.close();

                return;
            }
            else {
                cout << "Sorry, that's incorrect. What would you like to do?" << endl;
                cout << "1. Try again" << endl;
                cout << "2. Skip to the next question" << endl;
                int choice;
                cin >> choice;
                if (choice == 1) {
                    secondTry = true;
                }
                else if (choice == 2) {
                    cout << "Skipping to the next question..." << endl;
                    break;
                }
                else {
                    cout << "Invalid choice. Please enter 1 or 2." << endl;
                }
            }
        }

        // Update points for the next question
        points *= 10;

        // If the player answered correctly for the current question, reset consecutiveCorrect count
        if (consecutiveCorrect == 0) {
            points = 10; // Reset points to the initial value
        }
    }

    cout << "Congratulations! You won " << totalScore << " points." << endl;
}



// Function to sort and display player scores and ranks
void sort_score() {
    vector<string> playerNames;
    vector<int> playerScores;

    // Read in names and scores from "summary.txt"
    ifstream summaryFile("summary.txt");
    if (!summaryFile) {
        cerr << "Failed to open summary file." << endl;
        exit(1);
    }

    string name;
    int score;
    while (summaryFile >> name >> score) {
        playerNames.push_back(name);
        playerScores.push_back(score);
    }

    summaryFile.close();

    // Sort the scores and names accordingly
    vector<pair<int, string>> playerData;

    for (int i = 0; i < playerNames.size(); ++i) {
        playerData.push_back(make_pair(playerScores[i], playerNames[i]));
    }

    sort(playerData.begin(), playerData.end(), greater<pair<int, string>>());

    cout << "Top Player: " << playerData[0].second << " with a score of $" << playerData[0].first << endl;

    // Find the rank of the current player based on their score
    int currentScore = playerScores.back();
    int rank = 1;

    for (int i = 0; i < playerData.size(); ++i) {
        if (playerData[i].first > currentScore) {
            rank++;
        }
        else {
            break;
        }
    }

    cout << "Your rank: " << rank << " out of " << playerData.size() << endl;
}


// Main function
int main(int argc, char* argv[]) {
    if (argc != 4) {
        cout << "Usage: " << argv[0] << " <question_file> <answer_file> <seed>" << endl;
        return 1;
    }

    const string questionFile = argv[1];
    const string answerFile = argv[2];
    int seed = atoi(argv[3]);
    string playerName;

    cout << "Enter your name: ";
    cin >> playerName;

    srand(seed); // Seed srand with the provided integer

    vector<Question> allQuestions;
    vector<char> correctAnswers;

    read_questions(questionFile, allQuestions);
    read_answers(answerFile, correctAnswers);

    if (allQuestions.size() < 6) {
        cerr << "Not enough questions in the file." << endl;
        return 1;
    }

    vector<int> selectedIndices;
    vector<Question> selectedQuestions;
    int numQuestionsToSelect = 6;

    while (selectedIndices.size() < numQuestionsToSelect) {
        int randomIndex = rand() % allQuestions.size();

        // Check if the question has already been selected
        if (find(selectedIndices.begin(), selectedIndices.end(), randomIndex) == selectedIndices.end()) {
            selectedIndices.push_back(randomIndex);
            selectedQuestions.push_back(allQuestions[randomIndex]);
        }
    }

    // Shuffle the selected questions randomly
    random_shuffle(selectedQuestions.begin(), selectedQuestions.end());

    vector<char> selectedCorrectAnswers;
    for (int index : selectedIndices) {
        selectedCorrectAnswers.push_back(correctAnswers[index]);
    }

    play_game(selectedQuestions, selectedCorrectAnswers, playerName);

    return 0;
}
