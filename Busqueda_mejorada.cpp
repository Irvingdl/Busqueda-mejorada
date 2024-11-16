#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <chrono>

using namespace std;

// Estructura para almacenar los documentos con su puntaje
struct Document {
    string name;
    double score;
};

// Ordenar documentos por puntaje descendente
bool compareDocuments(const Document& a, const Document& b) {
    return a.score > b.score;
}

// Función para convertir a minúsculas
string toLowerCase(const string& str) {
    string lowerStr = str;
    transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}

// Leer el archivo de diccionario y obtener la posición del posting
streampos getPostingPosition(const string& dictionaryFile, const string& token) {
    ifstream inFile(dictionaryFile);
    if (!inFile) {
        cerr << "Error: No se pudo abrir el archivo de diccionario." << endl;
        exit(1);
    }

    string line, fileToken;
    streampos position = -1;
    while (getline(inFile, line)) {
        istringstream iss(line);
        iss >> fileToken;
        if (toLowerCase(fileToken) == toLowerCase(token)) {
            iss >> position; // Leer posición del posting
            break;
        }
    }

    inFile.close();
    return position;
}

// Leer los documentos asociados a un token desde el archivo de posting
vector<Document> getDocumentsFromPosting(const string& postingFile, streampos position) {
    ifstream inFile(postingFile, ios::binary);
    if (!inFile) {
        cerr << "Error: No se pudo abrir el archivo de postings." << endl;
        exit(1);
    }

    inFile.seekg(position);
    vector<Document> documents;
    string docName;
    double score;

    for (int i = 0; i < 10 && inFile >> docName >> score; ++i) {
        documents.push_back({docName, score});
    }

    inFile.close();
    return documents;
}

// Buscar tokens y mostrar el top 10 documentos
void retrieveTokens(const vector<string>& tokens, const string& dictionaryFile, const string& postingFile) {
    vector<Document> allResults;

    for (const auto& token : tokens) {
        streampos position = getPostingPosition(dictionaryFile, token);
        if (position != -1) {
            vector<Document> documents = getDocumentsFromPosting(postingFile, position);
            allResults.insert(allResults.end(), documents.begin(), documents.end());
        }
    }

    // Consolidar resultados y calcular top 10
    unordered_map<string, double> docScores;
    for (const auto& doc : allResults) {
        docScores[doc.name] += doc.score;
    }

    vector<Document> finalResults;
    for (const auto& [name, score] : docScores) {
        finalResults.push_back({name, score});
    }

    sort(finalResults.begin(), finalResults.end(), compareDocuments);

    cout << "Top 10 documentos relevantes:" << endl;
    for (size_t i = 0; i < min(finalResults.size(), size_t(10)); ++i) {
        cout << i + 1 << ". " << finalResults[i].name << " (Score: " << finalResults[i].score << ")" << endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cerr << "Uso: " << argv[0] << " <archivo_diccionario> <archivo_posting> <tokens...>" << endl;
        return 1;
    }

    string dictionaryFile = argv[1];
    string postingFile = argv[2];
    vector<string> tokens;

    for (int i = 3; i < argc; ++i) {
        tokens.push_back(argv[i]);
    }

    auto startTime = chrono::high_resolution_clock::now();

    // Realizar búsqueda
    retrieveTokens(tokens, dictionaryFile, postingFile);

    auto endTime = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = endTime - startTime;

    // Registrar tiempos en el log
    ofstream logFile("a13_matricula.txt", ios::app);
    if (logFile) {
        logFile << "Tokens: ";
        for (const auto& token : tokens) {
            logFile << token << " ";
        }
        logFile << ", Tiempo de búsqueda: " << elapsed.count() << " segundos" << endl;
        logFile.close();
    }

    return 0;
}
