// Transforma matrizes de adjacências (inclusive separadas por vírgulas)
// Na entrada dada por número de vértices e listagem de arestas (sem redundância)
// Propício para especificação da entrada do TP1 de Algorítmo do Gegê
// Utilitário criado por Marcos Dias para a saída do  https://graphonline.ru/pt/
// Versão inicial e tvz única do código:

//           -----> Se melhorar o código, manda pra mim por favor! ('saporra é útil): belmont.programmer@gmail.com

// Gerador de matrizes de adjacências simples:
// https://graphonline.ru/pt/?graph=TRtGsogddGgtIayX

// ==================================  USO ==================================================== //
/*
O código foi feito pensando em desvio de entrada e desvio de saída padrão.
Nada impede de utilizá-lo linha a linha, porém indica-se usar arquivos de entrada e saída
Ou seja:
          use < para apontar o arquivo contendo a matriz de adjacência (gerada ou escrita);
          use >(reescreve) e >> (append) para direcionar um arquivo de saída.
          caso o arquivo de saída não exista o sistema costuma criar um novo.

    ex: util < "entrada.in" > "saída.out"

    ou: util -m < "entrada.in" >> "saída.out"
            -r  (imprime matriz de adj sem vírgulas)
            >>  (força "append" (escrever no final do arquivo de saída))

** Utilize a flag "-m" para gerar uma matriz de adjcências "limpada" (sem vírgulas) ANTES do Grafo dado por arestas
Por padrão essa opção é desabilitada (propiciando o uso mais conviniente do arquivo de saída no redirecionamento pra entradado TP).
*/
// =============================================  USO ================================================= //

// Link de exemplo do primeiro grafo da especificação:
//http://graphonline.ru/pt/?graph=CYCYtUvgaOOGrsxJ
#include <cstdlib>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

string readAndClearLine(char clear) {
  string line;
  getline(cin, line);
  for (int i = 0; i < line.size(); i++) {
    if (line[i] == clear)
      line[i] = ' ';
  }

  return line;
}

vector<int> row(string line, vector<int>& v, bool verbose = false) {
  stringstream ss(line);
  string token;

  while (getline(ss, token, ' ')) {
    if (isdigit(token[0])) {
      int k = stoi(token) << ' ';
      v.emplace(v.end(), k);
      if (verbose) cout << k << ' ';
    }
  }
  if (verbose) cout << endl;

  return v;
}

int main(int charc, char** charv) {
  bool adjMatrix = false;
  if (charc > 1) {
    for (int i = 1; i < charc; i++) {
      if (charv[i][0] == '-') {
        switch (charv[i][1]) {
          case 'm':
            adjMatrix = true;  // Cospe a matriz de Adj no redirecionamento (só serve pra arquivo)
            break;

          default:
            break;
        }
      }
    }
  }
  vector<int> v;

  // cospe matriz de adjacências (quando saída é redirecionada) linha a linha
  set<set<int>> graphEdges;
  int id = 0;
  // row (string_limpa, vector<int> v, bool imprime_matriz_ou_Nâo
  v = row(readAndClearLine(','), v, adjMatrix);  // se adjMatrix = true, cospe linhas da matriz (só serve pra saída em arquivo)
  int n = v.size();

  for (int j = 0; j < n; j++) {
    if (v[j] == 1) {
      set<int> edge;
      edge.insert(j);
      edge.insert(id);
      graphEdges.insert(edge);
    }
  }
  id++;

  for (int i = 1; i < n; i++) {
    v.clear();
    v = row(readAndClearLine(','), v, adjMatrix);
    int m = v.size();
    for (int j = 0; j < m; j++) {
      if (v[j] == 1) {
        set<int> edge;
        edge.insert(j);
        edge.insert(id);
        graphEdges.insert(edge);
      }
    }
    id++;
  }

  // Saída no Grafo segundo a entrada da especificação
  if (adjMatrix) {
    cout << endl;
    cout << "Grafo: :" << endl;
    cout << endl;
  }
  cout << n << ' ' << graphEdges.size() << endl;
  for (set<int> s : graphEdges) {
    for (int x : s) {
      cout << x + 1 << ' ';
    }
    cout << endl;
  }
  // readAndClearLine(' ');
}