#pragma GCC optimize("-O3")
#pragma GCC optimize("inline")
#pragma GCC optimize("omit-frame-pointer")
#pragma GCC optimize("unroll-loops")

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

#include <ios>
#include <iostream>
#include <chrono>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>
#include <random>

using namespace std;
using namespace std::chrono;

high_resolution_clock::time_point start;
#define NOW high_resolution_clock::now()
#define TIME duration_cast<duration<double>>(NOW - start).count()

// The watch macro is one of the most useful tricks ever.
#define watch(x) cerr << (#x) << " is " << (x) << "\n"

constexpr int DEPTH = 50;
const string ACTION_TYPES[] = {"BREW", "CAST", "REST"};
constexpr int POPULATION_SIZE = 2000;
constexpr double TIME_LIMIT_FIRST_TURN = 0.85;
constexpr double TIME_LIMIT = 0.045;

int turn = 0;

// Function to generate random numbers in given range
int random_num(int start, int end) {
    int range = (end - start) + 1;
    int random_int = start + (rand() % range);
    return random_int;
}

class Recipe {
public:
    int id;
    int delta0;
    int delta1;
    int delta2;
    int delta3;
    int price;

    Recipe(int id, int delta0, int delta1, int delta2, int delta3, int price);
};

vector<Recipe *> recipes;
vector<int> recipesIds;
vector<int> playerSpellsIds;

class Spell {
public:
    int id;
    int delta0;
    int delta1;
    int delta2;
    int delta3;
    bool castable;
    bool sCastable;

    Spell(int id, int delta0, int delta1, int delta2, int delta3, bool castable);

    void reset();

    void printDebug();
};

class Player {
public:
    int id;
    int inv0;
    int sInv0;
    int inv1;
    int sInv1;
    int inv2;
    int sInv2;
    int inv3;
    int sInv3;
    int score;
    int sScore;
    vector<Spell *> spells;

    explicit Player(int id);

    void print();

    void play(const string &actionType, int actionId);

    void brew(int actionId);

    void cast(int actionId);

    void rest();

    void wait();

    void reset();

    int inventorySpace();

    float fitness();
};

Player *player;
Player *opponent;

class Individual {
public:
    int playerId;
    string actionTypes[DEPTH];
    int actionIds[DEPTH]{};
    float fitness = -INFINITY;

    explicit Individual(int playerId);

    void randomize();

    void simulate();

    Individual *crossover(Individual *individual);

    void mutate();

    void copy(Individual *individual);

    void printAction();

    void printDebug();
};

Recipe::Recipe(int id, int delta0, int delta1, int delta2, int delta3, int price) {
    this->id = id;
    this->delta0 = delta0;
    this->delta1 = delta1;
    this->delta2 = delta2;
    this->delta3 = delta3;
    this->price = price;
}

Spell::Spell(int id, int delta0, int delta1, int delta2, int delta3, bool castable) {
    this->id = id;
    this->delta0 = delta0;
    this->delta1 = delta1;
    this->delta2 = delta2;
    this->delta3 = delta3;
    this->castable = castable;
    this->sCastable = castable;
}

void Spell::reset() {
    this->castable = this->sCastable;
}

void Spell::printDebug() {
    cerr << "[" << this->delta0 << ", " << this->delta1 << ", " << this->delta2 << ", " << this->delta3 << "]" << "\n";
}

Player::Player(int id) {
    this->id = id;
    this->inv0 = 0;
    this->sInv0 = 0;
    this->inv1 = 0;
    this->sInv1 = 0;
    this->inv2 = 0;
    this->sInv2 = 0;
    this->inv3 = 0;
    this->sInv3 = 0;
    this->score = 0;
    this->sScore = 0;
}

void Player::print() {
    cerr << "[" << this->inv0 << ", " << this->inv1 << ", " << this->inv2 << ", " << this->inv3 << "]" << "\n";
}

void Player::play(const string &actionType, int actionId) {
    if (actionType == "BREW") {
        this->brew(actionId);
    } else if (actionType == "CAST") {
        this->cast(actionId);
    } else if (actionType == "REST") {
        this->rest();
    } else if (actionType == "WAIT") {
        this->wait();
    }
}

void Player::brew(int actionId) {
    Recipe *recipe;

    for (auto &i : recipes) {
        if (i->id == actionId) {
            recipe = i;
        }
    }

    if (this->inv0 + recipe->delta0 < 0 || this->inv1 + recipe->delta1 < 0 || this->inv2 + recipe->delta2 < 0
        || this->inv3 + recipe->delta3 < 0) {
        return;
    }

    this->inv0 += recipe->delta0;
    this->inv1 += recipe->delta1;
    this->inv2 += recipe->delta2;
    this->inv3 += recipe->delta3;
    this->score += recipe->price;
}

void Player::cast(int actionId) {
    Spell *spell;

    for (auto &i : this->spells) {
        if (i->id == actionId) {
            spell = i;
        }
    }

    if (!spell->castable) {
        return;
    }

    if (this->inventorySpace() < spell->delta0 + spell->delta1 + spell->delta2 + spell->delta3) {
        return;
    }

    if (this->inv0 + spell->delta0 < 0 || this->inv1 + spell->delta1 < 0 || this->inv2 + spell->delta2 < 0
        || this->inv3 + spell->delta3 < 0) {
        return;
    }

    this->inv0 += spell->delta0;
    this->inv1 += spell->delta1;
    this->inv2 += spell->delta2;
    this->inv3 += spell->delta3;
    spell->castable = false;
}

void Player::rest() {
    for (auto &spell : this->spells) {
        spell->castable = true;
    }
}

void Player::wait() {

}

void Player::reset() {
    this->inv0 = this->sInv0;
    this->inv1 = this->sInv1;
    this->inv2 = this->sInv2;
    this->inv3 = this->sInv3;
    this->score = this->sScore;

    for (auto &spell : this->spells) {
        spell->reset();
    }
}

int Player::inventorySpace() {
    return 10 - (this->inv0 + this->inv1 + this->inv2 + this->inv3);
}

float Player::fitness() {
    return this->score + this->inv1 + this->inv2 + this->inv3;
}

Individual::Individual(int playerId) {
    this->playerId = playerId;
}

void Individual::randomize() {
    for (int i = 0; i < DEPTH; i++) {
        string actionType = ACTION_TYPES[random_num(0, 2)];
        int actionId = 0;

        if (actionType == "BREW") {
            actionId = recipesIds.at(random_num(0, recipesIds.size() - 1));
        } else if (actionType == "CAST") {
            actionId = playerSpellsIds.at(random_num(0, playerSpellsIds.size() - 1));
        }

        this->actionTypes[i] = actionType;
        this->actionIds[i] = actionId;
    }
}

void Individual::simulate() {
    for (int i = 0; i < DEPTH; i++) {
        player->play(this->actionTypes[i], this->actionIds[i]);
    }

    this->fitness = player->fitness();
    player->reset();
}

Individual *Individual::crossover(Individual *individual) {
    auto *child = new Individual(this->playerId);

    for (int i = 0; i < DEPTH; i++) {
        if (random_num(0, 1)) {
            child->actionTypes[i] = this->actionTypes[i];
            child->actionIds[i] = this->actionIds[i];
        } else {
            child->actionTypes[i] = individual->actionTypes[i];
            child->actionIds[i] = individual->actionIds[i];
        }
    }

    return child;
}

void Individual::mutate() {
    for (int i = 0; i < DEPTH; i++) {
        if (random_num(0, 9) == 0) {
            string actionType = ACTION_TYPES[random_num(0, 2)];
            int actionId = 0;

            if (actionType == "BREW") {
                actionId = recipesIds.at(random_num(0, recipesIds.size() - 1));
            } else if (actionType == "CAST") {
                actionId = playerSpellsIds.at(random_num(0, playerSpellsIds.size() - 1));
            }

            this->actionTypes[i] = actionType;
            this->actionIds[i] = actionId;
        }
    }
}

void Individual::copy(Individual *individual) {
    for (int i = 0; i < DEPTH; i++) {
        this->actionTypes[i] = individual->actionTypes[i];
        this->actionIds[i] = individual->actionIds[i];
    }

    this->fitness = individual->fitness;
}

void Individual::printAction() {
    if (this->actionTypes[0] == "REST" || this->actionTypes[0] == "WAIT") {
        cout << this->actionTypes[0] << "\n";
    } else {
        cout << this->actionTypes[0] << " " << this->actionIds[0] << "\n";
    }
}

void Individual::printDebug() {
    for (int i = 0; i < DEPTH; i++) {
        cerr << this->actionTypes[i] << " " << this->actionIds[i] << "\n";
    }
}

bool operator<(const Individual &individual1, const Individual &individual2) {
    return individual1.fitness < individual2.fitness;
}

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/
int main() {
    player = new Player(0);
    opponent = new Player(1);

    auto *best = new Individual(0);

    // game loop
    while (true) {
        // the number of spells and recipes in play
        int actionCount;

        cin >> actionCount;
        cin.ignore();

        for (int i = 0; i < actionCount; i++) {
            // the unique ID of this spell or recipe
            int actionId;

            // in the first league: BREW; later: CAST, OPPONENT_CAST, LEARN, BREW
            string actionType;

            // tier-0 ingredient change
            int delta0;

            // tier-1 ingredient change
            int delta1;

            // tier-2 ingredient change
            int delta2;

            // tier-3 ingredient change
            int delta3;

            // the price in rupees if this is a potion
            int price;

            // in the first two leagues: always 0; later: the index in the tome if this is a tome spell, equal to the read-ahead tax;
            // For brews, this is the value of the current urgency bonus
            int tomeIndex;

            // in the first two leagues: always 0; later: the amount of taxed tier-0 ingredients you gain from learning this spell;
            // For brews, this is how many times you can still gain an urgency bonus
            int taxCount;

            // in the first league: always 0; later: 1 if this is a castable player spell
            bool castable;

            // for the first two leagues: always 0; later: 1 if this is a repeatable player spell
            bool repeatable;

            cin >> actionId >> actionType >> delta0 >> delta1 >> delta2 >> delta3 >> price >> tomeIndex >> taxCount
                >> castable >> repeatable;
            cin.ignore();

            if (actionType == "BREW") {
                recipesIds.emplace_back(actionId);
                auto *recipe = new Recipe(actionId, delta0, delta1, delta2, delta3, price);
                recipes.emplace_back(recipe);
            }

            if (actionType == "CAST") {
                playerSpellsIds.emplace_back(actionId);
                auto *spell = new Spell(actionId, delta0, delta1, delta2, delta3, castable);
                player->spells.emplace_back(spell);
            }

            if (actionType == "OPPONENT_CAST") {
                auto *spell = new Spell(actionId, delta0, delta1, delta2, delta3, castable);
                opponent->spells.emplace_back(spell);
            }
        }

        for (int i = 0; i < 2; i++) {
            // tier-0/1/2/3 ingredients in inventory
            int inv0;
            int inv1;
            int inv2;
            int inv3;

            // amount of rupees
            int score;

            cin >> inv0 >> inv1 >> inv2 >> inv3 >> score;
            cin.ignore();

            if (i == 0) {
                player->inv0 = inv0;
                player->sInv0 = inv0;
                player->inv1 = inv1;
                player->sInv1 = inv1;
                player->inv2 = inv2;
                player->sInv2 = inv2;
                player->inv3 = inv3;
                player->sInv3 = inv3;
                player->score = score;
                player->sScore = score;
            } else {
                opponent->inv0 = inv0;
                opponent->sInv0 = inv0;
                opponent->inv1 = inv1;
                opponent->sInv1 = inv1;
                opponent->inv2 = inv2;
                opponent->sInv2 = inv2;
                opponent->inv3 = inv3;
                opponent->sInv3 = inv3;
                opponent->score = score;
                opponent->sScore = score;
            }
        }

        // *************************************************************************************************************
        // <Genetic Evolution>
        // *************************************************************************************************************

        double limit = turn ? TIME_LIMIT : TIME_LIMIT_FIRST_TURN;

        vector<Individual *> playerPopulation;
        vector<Individual *> playerNewGeneration;

        playerPopulation.push_back(new Individual(0));
        playerPopulation[0]->randomize();
        playerPopulation[0]->simulate();

        best->copy(playerPopulation[0]);

        int generation = 1;

        // Generate starting population
        for (int i = 1; i < POPULATION_SIZE; i++) {
            playerPopulation.push_back(new Individual(0));
            playerPopulation[i]->randomize();
            playerPopulation[i]->simulate();

            if (playerPopulation[i]->fitness > best->fitness) {
                best->copy(playerPopulation[i]);
            }
        }

        while (TIME < limit) {
            sort(playerPopulation.begin(), playerPopulation.end());

            // Perform Elitism, that mean 10% of fittest population
            // goes to the next generation
            int s = (10 * POPULATION_SIZE) / 100;

            for (int i = 0; i < s; i++) {
                playerNewGeneration.push_back(playerPopulation[POPULATION_SIZE - 1 - i]);
            }

            // From 50% of fittest population, Individuals
            // will mate to produce offspring
            s = (90 * POPULATION_SIZE) / 100;

            for (int i = 0; i < s; i++) {
                int r = random_num(0, POPULATION_SIZE / 2);
                Individual *parent1 = playerPopulation[POPULATION_SIZE - 1 - r];

                r = random_num(0, POPULATION_SIZE / 2);
                Individual *parent2 = playerPopulation[POPULATION_SIZE - 1 - r];

                Individual *child = parent1->crossover(parent2);
                child->mutate();
                playerNewGeneration.push_back(child);
                child->simulate();

                if (child->fitness > best->fitness) {
                    best->copy(child);
                }
            }

            playerPopulation = playerNewGeneration;
            generation++;
        }

        // *************************************************************************************************************
        // </Genetic Evolution>
        // *************************************************************************************************************

        best->printAction();

        recipes.clear();
        recipes.shrink_to_fit();
        recipesIds.clear();
        recipesIds.shrink_to_fit();
        playerSpellsIds.clear();
        playerSpellsIds.shrink_to_fit();
        player->spells.clear();
        player->spells.shrink_to_fit();
        opponent->spells.clear();
        opponent->spells.shrink_to_fit();
        playerPopulation.clear();
        playerPopulation.shrink_to_fit();
        playerNewGeneration.clear();
        playerNewGeneration.shrink_to_fit();

        turn++;
    }
}