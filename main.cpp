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
const string ACTION_TYPES[] = {"BREW", "CAST", "LEARN", "REST"};
constexpr int TOME_SIZE = 6;
constexpr int RECIPES_SIZE = 5;
constexpr int POPULATION_SIZE = 200;
constexpr double TIME_LIMIT_FIRST_TURN = 0.85;
constexpr double TIME_LIMIT = 0.045;

int turn = 0;
int newSpellStartId = 86;

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
    int urgencyPrice;

    Recipe(int id, int delta0, int delta1, int delta2, int delta3, int price, int urgencyPrice = 0);

    void printDebug();
};

vector<Recipe *> recipes;
vector<Recipe *> sRecipes;
vector<Recipe *> allRecipes;
vector<Recipe *> sAllRecipes;

class Spell {
public:
    int id;
    int delta0;
    int delta1;
    int delta2;
    int delta3;
    bool castable;
    bool sCastable;
    bool repeatable;
    int taxValue;
    int taxGrab;

    Spell(int id, int delta0, int delta1, int delta2, int delta3, bool castable, bool repeatable, int taxValue,
          int taxGrab);

    void reset();

    void printDebug();
};

vector<Spell *> tome;
vector<Spell *> sTome;
vector<Spell *> allSpells;
vector<Spell *> sAllSpells;

class Individual {
public:
    int playerId;
    string actionTypes[DEPTH];
    int actionIds[DEPTH]{};
    float fitness = 0;

    explicit Individual(int playerId);

    void randomize();

    void simulate();

    Individual *crossover(Individual *individual);

    void mutate();

    void copy(Individual *individual);

    void printAction();

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
    vector<Spell *> sSpells;

    explicit Player(int id);

    void print();

    void play(Individual *individual, const string &actionType, int actionId);

    void brew(Individual *individual, int actionId);

    void cast(Individual *individual, int actionId);

    void learn(Individual *individual, int actionId);

    void rest(Individual *individual);

    void wait();

    void reset();

    int inventorySpace();

    float fitness();
};

Player *player;
Player *opponent;

Recipe::Recipe(int id, int delta0, int delta1, int delta2, int delta3, int price, int urgencyPrice) {
    this->id = id;
    this->delta0 = delta0;
    this->delta1 = delta1;
    this->delta2 = delta2;
    this->delta3 = delta3;
    this->price = price;
    this->urgencyPrice = urgencyPrice;
}

void Recipe::printDebug() {
    cerr << "[" << this->id << ", " << this->delta0 << ", " << this->delta1 << ", " << this->delta2 << ", "
         << this->delta3 << ", " << this->price << "]" << "\n";
}

Spell::Spell(int id, int delta0, int delta1, int delta2, int delta3, bool castable = true, bool repeatable = true,
             int taxValue = 0, int taxGrab = 0) {
    this->id = id;
    this->delta0 = delta0;
    this->delta1 = delta1;
    this->delta2 = delta2;
    this->delta3 = delta3;
    this->castable = castable;
    this->sCastable = castable;
    this->repeatable = repeatable;
    this->taxValue = taxValue;
    this->taxGrab = taxGrab;
}

void Spell::reset() {
    this->castable = this->sCastable;
}

void Spell::printDebug() {
    cerr << "[" << this->id << ", " << this->delta0 << ", " << this->delta1 << ", " << this->delta2 << ", "
         << this->delta3 << ", " << this->castable << ", " << this->repeatable << ", " << this->taxValue << ", "
         << this->taxGrab << "]" << "\n";
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

void Player::play(Individual *individual, const string &actionType, int actionId) {
    if (actionType == "BREW") {
        this->brew(individual, actionId);
    } else if (actionType == "CAST") {
        this->cast(individual, actionId);
    } else if (actionType == "LEARN") {
        this->learn(individual, actionId);
    } else if (actionType == "REST") {
        this->rest(individual);
    } else if (actionType == "WAIT") {
        this->wait();
    }
}

void Player::brew(Individual *individual, int actionId) {
    Recipe *recipe = recipes[0];
    int index = 0;

    for (int i = 0; i < RECIPES_SIZE; i++) {
        if (recipes[i]->id == actionId) {
            recipe = recipes[i];
            index = i;
        }
    }

    if (this->inv0 + recipe->delta0 < 0 || this->inv1 + recipe->delta1 < 0 || this->inv2 + recipe->delta2 < 0
        || this->inv3 + recipe->delta3 < 0) {
        individual->fitness -= 100;
        return;
    }

    this->inv0 += recipe->delta0;
    this->inv1 += recipe->delta1;
    this->inv2 += recipe->delta2;
    this->inv3 += recipe->delta3;
    this->score += recipe->price;
    recipes.erase(recipes.begin() + index);
    index = random_num(0, allRecipes.size() - 1);
    recipes.emplace_back(allRecipes[index]);
    allRecipes.erase(allRecipes.begin() + index);
}

void Player::cast(Individual *individual, int actionId) {
    Spell *spell = this->spells[0];

    for (auto &i : this->spells) {
        if (i->id == actionId) {
            spell = i;
        }
    }

    if (!spell->castable) {
        individual->fitness -= 100;
        return;
    }

    if (this->inventorySpace() < spell->delta0 + spell->delta1 + spell->delta2 + spell->delta3) {
        individual->fitness -= 100;
        return;
    }

    if (this->inv0 + spell->delta0 < 0 || this->inv1 + spell->delta1 < 0 || this->inv2 + spell->delta2 < 0
        || this->inv3 + spell->delta3 < 0) {
        individual->fitness -= 100;
        return;
    }

    this->inv0 += spell->delta0;
    this->inv1 += spell->delta1;
    this->inv2 += spell->delta2;
    this->inv3 += spell->delta3;
    spell->castable = false;
}

void Player::learn(Individual *individual, int actionId) {
    Spell *spell = tome[0];
    int index = 0;

    for (int i = 0; i < TOME_SIZE; i++) {
        if (tome[i]->id == actionId) {
            spell = tome[i];
            index = i;
        }
    }

    if (this->inv0 < spell->taxValue) {
        individual->fitness -= 100;
        return;
    }

    if (index == 0) {
        individual->fitness += 10;
    }

    for (int i = 0; i < index; i++) {
        tome[i]->taxGrab++;
    }

    this->inv0 += spell->taxGrab;
    int space = this->inventorySpace();

    if (space < 0) {
        this->inv0 += space;
    }

    this->spells.emplace_back(new Spell(newSpellStartId, spell->delta0, spell->delta1, spell->delta2, spell->delta3));
    newSpellStartId++;
    tome.erase(tome.begin() + index);
    index = random_num(0, allSpells.size() - 1);
    tome.emplace_back(allSpells[index]);
    allSpells.erase(allSpells.begin() + index);
}

void Player::rest(Individual *individual) {
    bool restable = false;

    for (auto &spell : this->spells) {
        if (!spell->castable) {
            spell->castable = true;
            restable = true;
        }
    }

    if (!restable) {
        individual->fitness -= 5;
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
    this->spells = this->sSpells;
    tome = sTome;
    allSpells = sAllSpells;
    recipes = sRecipes;
    allRecipes = sAllRecipes;
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
        string actionType = ACTION_TYPES[random_num(0, 3)];
        int actionId = 0;

        if (actionType == "BREW") {
            actionId = recipes[random_num(0, RECIPES_SIZE - 1)]->id;
        } else if (actionType == "CAST") {
            actionId = player->spells[random_num(0, player->spells.size() - 1)]->id;
        } else if (actionType == "LEARN") {
            actionId = tome[random_num(0, TOME_SIZE - 1)]->id;
        }

        this->actionTypes[i] = actionType;
        this->actionIds[i] = actionId;
    }
}

void Individual::simulate() {
    for (int i = 0; i < DEPTH; i++) {
        player->play(this, this->actionTypes[i], this->actionIds[i]);
    }

    this->fitness += player->fitness();
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
            string actionType = ACTION_TYPES[random_num(0, 3)];
            int actionId = 0;

            if (actionType == "BREW") {
                actionId = recipes[random_num(0, RECIPES_SIZE - 1)]->id;
            } else if (actionType == "CAST") {
                actionId = player->spells[random_num(0, player->spells.size() - 1)]->id;
            } else if (actionType == "LEARN") {
                actionId = tome[random_num(0, TOME_SIZE - 1)]->id;
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
//    individual->printDebug();
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
//    cerr << this->actionTypes[0] << " " << this->actionIds[0] << "\n";
    cerr << this->fitness << "\n";
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

    allSpells.emplace_back(new Spell(0, -3, 0, 0, 1));
    allSpells.emplace_back(new Spell(1, 3, -1, 0, 0));
    allSpells.emplace_back(new Spell(2, 1, 1, 0, 0));
    allSpells.emplace_back(new Spell(3, 0, 0, 1, 0));
    allSpells.emplace_back(new Spell(4, 3, 0, 0, 0));
    allSpells.emplace_back(new Spell(5, 2, 3, -2, 0));
    allSpells.emplace_back(new Spell(6, 2, 1, -2, 1));
    allSpells.emplace_back(new Spell(7, 3, 0, 1, -1));
    allSpells.emplace_back(new Spell(8, 3, -2, 1, 0));
    allSpells.emplace_back(new Spell(9, 2, -3, 2, 0));
    allSpells.emplace_back(new Spell(10, 2, 2, 0, -1));
    allSpells.emplace_back(new Spell(11, -4, 0, 2, 0));
    allSpells.emplace_back(new Spell(12, 2, 1, 0, 0));
    allSpells.emplace_back(new Spell(13, 4, 0, 0, 0));
    allSpells.emplace_back(new Spell(14, 0, 0, 0, 1));
    allSpells.emplace_back(new Spell(15, 0, 2, 0, 0));
    allSpells.emplace_back(new Spell(16, 1, 0, 1, 0));
    allSpells.emplace_back(new Spell(17, -2, 0, 1, 0));
    allSpells.emplace_back(new Spell(18, -1, -1, 0, 1));
    allSpells.emplace_back(new Spell(19, 0, 2, -1, 0));
    allSpells.emplace_back(new Spell(20, 2, -2, 0, 1));
    allSpells.emplace_back(new Spell(21, -3, 1, 1, 0));
    allSpells.emplace_back(new Spell(22, 0, 2, -2, 1));
    allSpells.emplace_back(new Spell(23, 1, -3, 1, 1));
    allSpells.emplace_back(new Spell(24, 0, 3, 0, -1));
    allSpells.emplace_back(new Spell(25, 0, -3, 0, 2));
    allSpells.emplace_back(new Spell(26, 1, 1, 1, -1));
    allSpells.emplace_back(new Spell(27, 1, 2, -1, 0));
    allSpells.emplace_back(new Spell(28, 4, 1, -1, 0));
    allSpells.emplace_back(new Spell(29, -5, 0, 0, 2));
    allSpells.emplace_back(new Spell(30, -4, 0, 1, 1));
    allSpells.emplace_back(new Spell(31, 0, 3, 2, -2));
    allSpells.emplace_back(new Spell(32, 1, 1, 3, -2));
    allSpells.emplace_back(new Spell(33, -5, 0, 3, 0));
    allSpells.emplace_back(new Spell(34, -2, 0, -1, 2));
    allSpells.emplace_back(new Spell(35, 0, 0, -3, 3));
    allSpells.emplace_back(new Spell(36, 0, -3, 3, 0));
    allSpells.emplace_back(new Spell(37, -3, 3, 0, 0));
    allSpells.emplace_back(new Spell(38, -2, 2, 0, 0));
    allSpells.emplace_back(new Spell(39, 0, 0, -2, 2));
    allSpells.emplace_back(new Spell(40, 0, -2, 2, 0));
    allSpells.emplace_back(new Spell(41, 0, 0, 2, -1));

    allRecipes.emplace_back(new Recipe(42, 2, 2, 0, 0, 6));
    allRecipes.emplace_back(new Recipe(43, 3, 2, 0, 0, 7));
    allRecipes.emplace_back(new Recipe(44, 0, 4, 0, 0, 8));
    allRecipes.emplace_back(new Recipe(45, 2, 0, 2, 0, 8));
    allRecipes.emplace_back(new Recipe(46, 2, 3, 0, 0, 8));
    allRecipes.emplace_back(new Recipe(47, 3, 0, 2, 0, 9));
    allRecipes.emplace_back(new Recipe(48, 0, 2, 2, 0, 10));
    allRecipes.emplace_back(new Recipe(49, 0, 5, 0, 0, 10));
    allRecipes.emplace_back(new Recipe(50, 2, 0, 0, 2, 10));
    allRecipes.emplace_back(new Recipe(51, 2, 0, 3, 0, 11));
    allRecipes.emplace_back(new Recipe(52, 3, 0, 0, 2, 11));
    allRecipes.emplace_back(new Recipe(53, 0, 0, 4, 0, 12));
    allRecipes.emplace_back(new Recipe(54, 0, 2, 0, 2, 12));
    allRecipes.emplace_back(new Recipe(55, 0, 3, 2, 0, 12));
    allRecipes.emplace_back(new Recipe(56, 0, 2, 3, 0, 13));
    allRecipes.emplace_back(new Recipe(57, 0, 0, 2, 2, 14));
    allRecipes.emplace_back(new Recipe(58, 0, 3, 0, 2, 14));
    allRecipes.emplace_back(new Recipe(59, 2, 0, 0, 3, 14));
    allRecipes.emplace_back(new Recipe(60, 0, 0, 5, 0, 15));
    allRecipes.emplace_back(new Recipe(61, 0, 0, 0, 4, 16));
    allRecipes.emplace_back(new Recipe(62, 0, 2, 0, 3, 16));
    allRecipes.emplace_back(new Recipe(63, 0, 0, 3, 2, 17));
    allRecipes.emplace_back(new Recipe(64, 0, 0, 2, 3, 18));
    allRecipes.emplace_back(new Recipe(65, 0, 0, 0, 5, 20));
    allRecipes.emplace_back(new Recipe(66, 2, 1, 0, 1, 9));
    allRecipes.emplace_back(new Recipe(67, 0, 2, 1, 1, 12));
    allRecipes.emplace_back(new Recipe(68, 1, 0, 2, 1, 12));
    allRecipes.emplace_back(new Recipe(69, 2, 2, 2, 0, 13));
    allRecipes.emplace_back(new Recipe(70, 2, 2, 0, 2, 15));
    allRecipes.emplace_back(new Recipe(71, 2, 0, 2, 2, 17));
    allRecipes.emplace_back(new Recipe(72, 0, 2, 2, 2, 19));
    allRecipes.emplace_back(new Recipe(73, 1, 1, 1, 1, 12));
    allRecipes.emplace_back(new Recipe(74, 3, 1, 1, 1, 14));
    allRecipes.emplace_back(new Recipe(75, 1, 3, 1, 1, 16));
    allRecipes.emplace_back(new Recipe(76, 1, 1, 3, 1, 18));
    allRecipes.emplace_back(new Recipe(77, 1, 1, 1, 3, 20));

    vector<Individual *> playerPopulation;
    vector<Individual *> playerNewGeneration;

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
                auto *recipe = new Recipe(actionId, delta0, delta1, delta2, delta3, price, tomeIndex);
                recipes.emplace_back(recipe);
                allRecipes.erase(allRecipes.begin() + i);
            }

            if (actionType == "CAST") {
                auto *spell = new Spell(actionId, delta0, delta1, delta2, delta3, castable, false, 0, 0);
                player->spells.emplace_back(spell);
                player->sSpells.emplace_back(spell);
            }

            if (actionType == "LEARN") {
                auto *spell = new Spell(actionId, delta0, delta1, delta2, delta3, castable, repeatable, tomeIndex,
                                        taxCount);
                tome.emplace_back(spell);
                allSpells.erase(allSpells.begin() + i);
            }

            if (actionType == "OPPONENT_CAST") {
                auto *spell = new Spell(actionId, delta0, delta1, delta2, delta3, castable, false, 0, 0);
                opponent->spells.emplace_back(spell);
                opponent->sSpells.emplace_back(spell);
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

        sTome = tome;
        sAllSpells = allSpells;
        sRecipes = recipes;
        sAllRecipes = allRecipes;

        recipes.erase(recipes.begin());

        for (int i = 0; i < recipes.size() - 1; i++) {
            recipes[i]->printDebug();
        }

        cerr << "\n";

        for (int i = 0; i < sRecipes.size() - 1; i++) {
            sRecipes[i]->printDebug();
        }

        // *************************************************************************************************************
        // <Genetic Evolution>
        // *************************************************************************************************************

        double limit = turn ? TIME_LIMIT : TIME_LIMIT_FIRST_TURN;

        playerPopulation.push_back(new Individual(0));
        playerPopulation[0]->randomize();
        playerPopulation[0]->simulate();

        auto *best = new Individual(0);
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

        allRecipes.clear();
        allRecipes.shrink_to_fit();
//        sAllRecipes.clear();
//        sAllRecipes.shrink_to_fit();
        allSpells.clear();
        allSpells.shrink_to_fit();
//        sAllSpells.clear();
//        sAllSpells.shrink_to_fit();
        recipes.clear();
        recipes.shrink_to_fit();
//        sRecipes.clear();
//        sRecipes.shrink_to_fit();
        player->spells.clear();
        player->spells.shrink_to_fit();
//        player->sSpells.clear();
//        player->sSpells.shrink_to_fit();
        tome.clear();
        tome.shrink_to_fit();
//        sTome.clear();
//        sTome.shrink_to_fit();
        opponent->spells.clear();
        opponent->spells.shrink_to_fit();
//        opponent->sSpells.clear();
//        opponent->sSpells.shrink_to_fit();
        playerPopulation.clear();
        playerPopulation.shrink_to_fit();
        playerNewGeneration.clear();
        playerNewGeneration.shrink_to_fit();

        turn++;
    }
}