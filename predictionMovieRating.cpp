#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <ctime>
#include <map>
#include <utility>
#include <vector>
#include <algorithm>

using namespace std;

const int MAX_MOVIE_ID = 7335;
const int MAX_USER_ID = 35190;
const string INPUT_FILE = "train.csv";
const string OUTPUT_FILE = "submission.csv";
const string TEST_FILE = "test.txt";
const int MOST_SIMILAR_LIMIT = 100; // TODO: k değerini düşün
const float INVALID_VALUE = -1.0;

struct TestPair {
    int testId;
    int userId;
    int itemId;
};

class SortMap {
private:
    map<int, float> sortMap;

public:
    SortMap() {
    }

    static bool sortByVal(const pair<int, float> &a, const pair<int, float> &b) {
        return (a.second > b.second);
    }

    // kullanıcıların ve filmlerin top 10 sıralaması için kullandığmız integer bazlı increment
    void increment(int id) {
        if (sortMap.find(id) == sortMap.end()) {
            sortMap[id] = 1;
        } else {
            sortMap[id]++;
        }
    }

    // cosine hesaplarken sıralama için kullanacğımız float bazlı push
    void push(int id, float cosineValue) {
        sortMap[id] = cosineValue;
    }

    void printMaxElements(int count) {
        vector<pair<int, float>> v = this->getSortedPairs();

        for (int j = 0; j < count; j++) {
            cout << v[j].first << ", " << v[j].second << endl;
        }
    }

    vector<pair<int, float> > getSortedPairs() {
        map<int, float>::iterator itr;
        vector<pair<int, float> > v;
        for (itr = sortMap.begin(); itr != sortMap.end(); itr++) {
            v.emplace_back(itr->first, itr->second);
        }
        sort(v.begin(), v.end(), sortByVal);

        return v;
    }
};

class TrainData {
private:
    int column;
    int row;
    float **matrix;
public:
    TrainData() {

    };

    TrainData(int c, int r) {
        column = c;
        row = r;

        matrix = new float *[column];
        for (int c = 0; c < column; ++c){
            matrix[c] = new float[row];
            for (int r = 0; r < row; ++r){
                matrix[c][r] = INVALID_VALUE;
            }
        }
    }

    void setRating(int userId, int itemId, float rating) {
        matrix[userId][itemId] = rating;
    }

    float getRating(int userId, int itemId) {
        return matrix[userId][itemId];
    }

    int getColumn(){
        return column;
    }

    int getRow(){
        return row;
    }
};

class Predictor {
private:
    TrainData trainData;
public:
    Predictor(TrainData data) {
        trainData = data;
    }

    float run(int testUserId, int testItemId) {
        float consineSim;
        SortMap similarities;

        // bu movie'ye oy veren userlara bakıyoruz
        for(int userId = 0; userId < trainData.getColumn(); userId++){

            // Kendisi ile similarity hesaplamıyoruz
            if(userId == testUserId) {
                continue;
            }

            // eğer oylama yoksa taramaya devam
            if(trainData.getRating(userId, testItemId) == INVALID_VALUE) {
                continue;
            }

            /*
             * cosine hesaplayıp bir veri yapısına atama yapmamız lazım.
             * daha önce kullandığımız sort mekanizmasını kullanmak için burda bir sortmap oluşturacağız.
             * tüm similarity hesaplamalırını bu map içinde tutacağız ve en son sıralayıp ilk 100 tanesinin ortalamasını hesaplaycağız.
            */
            consineSim = calculateSimilarity(testUserId, userId);

            // eger ortak filmleri yoksa consine -1 donuyor
            if (consineSim == INVALID_VALUE) {
                continue;
            }

            similarities.push(userId, consineSim);
        }

        vector<pair<int, float>> mostSimilarUsers = similarities.getSortedPairs();

        mostSimilarUsers.resize(min(MOST_SIMILAR_LIMIT, (int) mostSimilarUsers.size()));

        return calculateRating(mostSimilarUsers, testItemId);
    }

    float calculateSimilarity(int testUserId, int trainUserId) {
        int commonMovies = 0;
        float dotProduct = 0, trainSquare = 0, testSquare = 0;

        for (int itemId = 0; itemId < trainData.getRow(); itemId++) {
            float testRating = trainData.getRating(testUserId, itemId);
            float trainRating = trainData.getRating(trainUserId, itemId);

            if (testRating == INVALID_VALUE || trainRating == INVALID_VALUE) {
                continue;
            }

            commonMovies++;
            dotProduct += testRating * trainRating;

            testSquare += pow(testRating, 2);
            trainSquare += pow(trainRating, 2);
        }

        // eger ortak fılmleri yoksa consine sim degerlendirmemiz lazım
        if (commonMovies == 0) {
            return INVALID_VALUE;
        }

        // if user gave only zero rating
        if (testSquare == 0 || trainSquare == 0) {
            return INVALID_VALUE;
        }

        return dotProduct / (sqrt(testSquare) * sqrt(trainSquare));
    }

    double calculateRating(vector<pair<int, float>> mostSimilarUsers, int itemId){
        float sum = 0, simSum = 0;

        for (auto pair = mostSimilarUsers.begin(); pair != mostSimilarUsers.end(); pair++) {
            sum += trainData.getRating(pair->first, itemId);
        }

        return sum / mostSimilarUsers.size();
    }

};

vector<TestPair> readTestData() {
    ifstream testFile(TEST_FILE);
    string line;
    string testId;
    string userId;
    string itemId;
    vector<TestPair> testData;

    // Discard column names line
    getline(testFile, line);

    while (getline(testFile, line)) {
        stringstream str(line);
        getline(str, testId, ',');
        getline(str, userId, ',');
        getline(str, itemId, ',');
        TestPair pair;
        pair.testId = stoi(testId);
        pair.userId = stoi(userId);
        pair.itemId = stoi(itemId);
        testData.emplace_back(pair);
    }

    return testData;
}

int main() {
    clock_t start = clock();

    SortMap userMaxList;
    SortMap itemMaxList;
    TrainData trainData(MAX_USER_ID, MAX_MOVIE_ID);

    ifstream trainFile(INPUT_FILE, ios::in);

    string line;
    string userId;
    string itemId;
    string rating;

    // Discard column names line
    getline(trainFile, line);

    while (getline(trainFile, line)) {
        stringstream str(line);
        getline(str, userId, ',');
        getline(str, itemId, ',');
        getline(str, rating, ',');
        int userIdVal = stoi(userId);
        int itemIdVal = stoi(itemId);
        float ratingVal = stof(rating);
        trainData.setRating(userIdVal, itemIdVal, ratingVal);
        userMaxList.increment(userIdVal);
        itemMaxList.increment(itemIdVal);
    }
    trainFile.close();

    cout << "Top 10 users" << endl;
    cout << "------------" << endl;
    userMaxList.printMaxElements(10);

    cout << endl;

    cout << "Top 10 movies" << endl;
    cout << "------------" << endl;
    itemMaxList.printMaxElements(10);

    vector<TestPair> testData = readTestData();

    Predictor predictor(trainData);

    ofstream outFile(OUTPUT_FILE);
    outFile << "ID,Predicted" << endl;
    for (auto itr = testData.begin(); itr != testData.end(); itr ++) {
        float prediction = predictor.run(itr->userId, itr->itemId);
        outFile << itr->testId << "," << prediction << endl;
    }
    outFile.close();

    clock_t end = clock();
    cout << "Elapsed time: " << (double) (end - start) / CLOCKS_PER_SEC << " seconds" << endl;
    return 0;
}

