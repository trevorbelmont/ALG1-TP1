#include <iostream>
#include <set>
#include <vector>
// ¬¬ por enquanto assumimos que 'A' = 0ª letra do alfabeto (sem offsets)
using namespace std;

// O TAD do Grafo
struct Vertex {
  int id;
  set<int> adj;  // lista de adjacências
};

class Graph {
 public:
  int size;
  Vertex* v;

  Graph(int tam) {
    size = tam;
    v = new Vertex[size];
    // Atribui as ids de cada vértice
    for (int i = 0; i < size; i++) {
      v[i].id = i;
    }
  }

  // Cria arestas na lista de adjacência que representa o grafo (função redundante dado que o o é um grafo não direcionado)
  bool makeEdge(int a, int b) {
    if ((a >= 0 && a < size) && (b >= 0 && b < size)) {  // o enunciado do TP não permite a = b
      v[a].adj.insert(b);
      v[b].adj.insert(a);
      return true;
    } else {
      cout << "Invalid vertex in bool makeEdge(int a, int b)!" << endl;
      cout << "(a,b,size) = (" << a << ',' << b << ',' << size << ')' << endl;
      return false;
    }
  }

  void print() {
    for (int i = 0; i < size; i++) {
      cout << '[' << v[i].id << ']';
      /*      FORMA COMPLETA
      for (set<int>::iterator it = v[i].adj.begin(); it != v[i].adj.end(); it++) {
        cout << ' ' << *it;
      }
      */

      for (int x : v[i].adj) {
        cout << ' ' << x;
      }
      cout << endl;
    }
  }
};

int main() {
  int N, M;

  cin >> N >> M;

  Graph G(N);

  for (int i = 0; i < M; i++) {
    int a, b;  // fora do loop?? ¬
    cin >> a >> b;

    G.makeEdge(a - 1, b - 1);
  }

  G.print();
}