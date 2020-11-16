#pragma GCC optimize("-O3")
#pragma GCC optimize("inline")
#pragma GCC optimize("omit-frame-pointer")
#pragma GCC optimize("unroll-loops")

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

// The watch macro is one of the most useful tricks ever.
#define watch(x) cerr << (#x) << " is " << (x) << "\n"

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

class Player {
public:
    int id;
    int inv0;
    int inv1;
    int inv2;
    int inv3;
    int score;

    Player(int id, int inv0, int inv1, int inv2, int inv3, int score);

    double evaluateRecipe(Recipe *recipe);

    double evaluate();
};

Recipe::Recipe(int id, int delta0, int delta1, int delta2, int delta3, int price) {
    this->id = id;
    this->delta0 = delta0;
    this->delta1 = delta1;
    this->delta2 = delta2;
    this->delta3 = delta3;
    this->price = price;
}

Player::Player(int id, int inv0, int inv1, int inv2, int inv3, int score) {
    this->id = id;
    this->inv0 = inv0;
    this->inv1 = inv1;
    this->inv2 = inv2;
    this->inv3 = inv3;
    this->score = score;
}

double Player::evaluateRecipe(Recipe *recipe) {
    if (this->inv0 - recipe->delta0 >= 0 && this->inv1 - recipe->delta1 >= 0 && this->inv2 - recipe->delta2 >= 0
        && this->inv3 - recipe->delta3 >= 0) {
        return recipe->price;
    }

    return 0;
}

double Player::evaluate() {
    return 0;
}

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/
int main() {
    auto *player = new Player(0, 0, 0, 0, 0, 0);
    auto *opponent = new Player(1, 0, 0, 0, 0, 0);
    vector<Recipe *> recipes;

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
                auto *recipe = new Recipe(actionId, delta0, delta1, delta2, delta3, price);
                recipes.emplace_back(recipe);
            }
        }

        for (int i = 0; i < 2; i++) {
            // tier-0 ingredients in inventory
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
                player->inv1 = inv1;
                player->inv2 = inv2;
                player->inv3 = inv3;
                player->score = score;
            } else {
                opponent->inv0 = inv0;
                opponent->inv1 = inv1;
                opponent->inv2 = inv2;
                opponent->inv3 = inv3;
                opponent->score = score;
            }
        }

        double maxRecipeValue = 0;
        int brewId = 0;

        for (int i = 0; i < recipes.size(); i++) {
            auto *recipe = recipes.at(i);
            double recipeValue = player->evaluateRecipe(recipe);
            watch(recipeValue);

            if (recipeValue > maxRecipeValue) {
                maxRecipeValue = recipeValue;
                brewId = recipe->id;
            }
        }

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;

        // in the first league: BREW <id> | WAIT; later: BREW <id> | CAST <id> [<times>] | LEARN <id> | REST | WAIT
        cout << "BREW " << brewId << endl;

        recipes.clear();
    }
}