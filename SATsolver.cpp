#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <vector>
using namespace std;

#define UNDEF -1
#define TRUE 1
#define FALSE 0

uint numVars;
uint numClauses;
vector<vector<int> > clauses;
vector<int> model;
vector<int> modelStack;
uint indexOfNextLitToPropagate;
uint decisionLevel;

vector<vector <int> > clausesWithNegatedLiteral;

void readClauses( ){
    // Skip comments
    char c = cin.get();
    while (c == 'c') {
        while (c != '\n') c = cin.get();
        c = cin.get();
    }  
    // Read "cnf numVars numClauses"
    string aux;
    cin >> aux >> numVars >> numClauses;
    clauses.resize(numClauses);
    clausesWithNegatedLiteral.resize(numVars+1);
    // Read clauses
    for (uint i = 0; i < numClauses; ++i) {
        int lit;
        cout << "Reading clause " << i << ": ";
        while (cin >> lit and lit != 0){
            cout << lit << ", ";
            clauses[i].push_back(lit);
            if (lit < 0) clausesWithNegatedLiteral[-lit].push_back(i);
            clausesWithNegatedLiteral[0].push_back(i); //totes les clausules estan a [0] per la primera iteracio
        }
        cout << "done." << endl;
    }    
}



int currentValueInModel(int lit){
    if (lit >= 0) return model[lit];
    else {
        if (model[-lit] == UNDEF) return UNDEF;
        else return 1 - model[-lit];
    }
}


void setLiteralToTrue(int lit){
    modelStack.push_back(lit);
    if (lit > 0) model[lit] = TRUE;
    else model[-lit] = FALSE;		
}


bool propagateGivesConflict (int decisionLit) {
    while ( indexOfNextLitToPropagate < modelStack.size() ) {
        ++indexOfNextLitToPropagate;
        cout << "There's " << clausesWithNegatedLiteral[decisionLit].size() << " clauses with " << decisionLit << " negated:" << endl;
        for (uint i = 0; i < clausesWithNegatedLiteral[decisionLit].size(); ++i) {
            cout << "    Checking the clause in the " << i << " position, number " << clausesWithNegatedLiteral[decisionLit][i] << ": "<< endl;
            bool someLitTrue = false;
            int numUndefs = 0;
            int lastLitUndef = 0;
            for (uint k = 0; not someLitTrue and k < clauses[clausesWithNegatedLiteral[decisionLit][i]].size(); ++k){
                int val = currentValueInModel(clauses[clausesWithNegatedLiteral[decisionLit][i]][k]);
                cout << "        " << clauses[clausesWithNegatedLiteral[decisionLit][i]][k] << " is ";
                if (val == TRUE){
                    someLitTrue = true;
                    cout << "TRUE, we are done here.";
                }
                else if (val == UNDEF){
                    ++numUndefs;
                    lastLitUndef = clauses[clausesWithNegatedLiteral[decisionLit][i]][k];
                    cout << "UNDEF, we continue.";
                }
                else cout << "FALSE, we continue";
                cout << endl;
            }
            if (not someLitTrue and numUndefs == 0) {
                cout << "            CONFLICT! All lits false!" << endl;
                return true; // conflict! all lits false
            }
            else if (not someLitTrue and numUndefs == 1){
                setLiteralToTrue(lastLitUndef);
                cout << "            Only one UNDEF, " << lastLitUndef << ", which we set to true." << endl;
            }
            else cout << "            No conflict" << endl;
        }
    }
    return false;
}


void backtrack(){
    uint i = modelStack.size() -1;
    int lit = 0;
    while (modelStack[i] != 0){ // 0 is the DL mark
        lit = modelStack[i];
        model[abs(lit)] = UNDEF;
        modelStack.pop_back();
        --i;
    }
    // at this point, lit is the last decision
    modelStack.pop_back(); // remove the DL mark
    --decisionLevel;
    indexOfNextLitToPropagate = modelStack.size();
    setLiteralToTrue(-lit);  // reverse last decision
}


// Heuristic for finding the next decision literal:
int getNextDecisionLiteral(){
    for (uint i = 1; i <= numVars; ++i) // stupid heuristic:
        if (model[i] == UNDEF) return i;  // returns first UNDEF var, positively
        return 0; // reurns 0 when all literals are defined
}

void checkmodel(){
    for (int i = 0; i < numClauses; ++i){
        bool someTrue = false;
        for (int j = 0; not someTrue and j < clauses[i].size(); ++j)
            someTrue = (currentValueInModel(clauses[i][j]) == TRUE);
        if (not someTrue) {
            cout << "Error in model, clause " << i << " is not satisfied:";
            for (int j = 0; j < clauses[i].size(); ++j) cout << clauses[i][j] << " ";
            cout << endl;
            exit(1);
        }
    }  
}

int main(){ 
    readClauses(); // reads numVars, numClauses and clauses
    model.resize(numVars+1,UNDEF);
    indexOfNextLitToPropagate = 0;  
    decisionLevel = 0;
    
    // Take care of initial unit clauses, if any
    for (uint i = 0; i < numClauses; ++i)
        if (clauses[i].size() == 1) {
            int lit = clauses[i][0];
            int val = currentValueInModel(lit);
            if (val == FALSE) {cout << "UNSATISFIABLE" << endl; return 10;}
            else if (val == UNDEF) setLiteralToTrue(lit);
        }
        
        int decisionLit = 0;
        // DPLL algorithm
        while (true) {
            while ( propagateGivesConflict(decisionLit) ) {
                if ( decisionLevel == 0) { cout << "UNSATISFIABLE" << endl; return 10; }
                backtrack();
            }
            decisionLit = getNextDecisionLiteral();
            //for (int i = 0; i<decisionLevel; ++i) cout << "  ";
            cout << "Propagating literal " << decisionLit << endl;
            if (decisionLit == 0) {
                for (int i = 1; i <= numVars; ++i) cout << "Value of " << i << " is " << currentValueInModel(i) << '.' << endl;
                checkmodel(); 
                cout << "SATISFIABLE" << endl; 
                return 20; }
            // start new decision level:
            modelStack.push_back(0);  // push mark indicating new DL
            ++indexOfNextLitToPropagate;
            ++decisionLevel;
            setLiteralToTrue(decisionLit);    // now push decisionLit on top of the mark
        }
}  
