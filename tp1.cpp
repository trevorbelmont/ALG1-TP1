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

  vector<bool> explored;
  vector<int> open, min;  // open = tempo de entrada (abertura na pilha), min = min(open[v],open[vizinhos de v])
  // Em outras palavras: min de um vértice   é o mínimo entre os tempos de abertura de um vértice e seus vizinhos (recursivamente)

  Graph(int tam) {
    size = tam;
    v = new Vertex[size];
    // Atribui as ids de cada vértice
    for (int i = 0; i < size; i++) {
      v[i].id = i;
    }
    // Inicializa todas as variáveis auxiliares
    explored.assign(size, false);
    open.assign(size, -1);
    min.assign(size, -1);
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
      //       FORMA BONITA
      cout << "Lista de Adjacência:" << endl;
      for (int x : v[i].adj) {
        cout << ' ' << x;
      }
      cout << endl;
    }
    for (int i = 0; i < size; i++) {
      cout << "~[" << i << ']' << ' ' << explored[i] << ' ' << open[i] << ' ' << min[i] << endl;
    }
  }

  set<int>* dfs(int w, int parent = -1, int t = 0, set<int>* cutSet = nullptr) {
    set<int>* cutPoints;  // ||

    if (t != 0) {
      cutPoints = cutSet;
    } else {
      cutPoints = new set<int>;
    }

    explored[w] = true;
    open[w] = min[w] = t++;
    int children = 0;

    for (int u : v[w].adj) {
      if (u == parent) continue;                          // se a aresta volta pro pai, ignora.
      if (explored[u] == true) {                          // se o vizinho já foi visitado na DFS
        min[w] = (min[w] <= open[u]) ? min[w] : open[u];  // min[v] = mínimo(min[v], open[u])
      } else {                                            // se o vizinho ainda não foi visitado
        dfs(u, w, t, cutPoints);
        min[w] = (min[w] <= min[u]) ? min[w] : min[u];
        if (min[u] >= open[w] && parent != -1) {
          cout << " ============== IS c u t p o i n t (" << w << ") ============" << endl;
          cutPoints->insert(w);
        } else {
          cout << "NOT CUTPOINT" << endl;
        }
        children++;
      }
    }
    if (parent == -1 && children > 1) {  // se é o vértice raiz da DFS e tem mais que um filho
      cout << " ============== IS c u t p o i n t (" << v << ") ============" << endl;
    }
    return cutPoints;
  }
};

Graph loadGraph() {
  int N, M;

  cin >> N >> M;

  Graph G(N);

  int a, b;  // fora do loop?? ¬
  for (int i = 0; i < M; i++) {
    cin >> a >> b;

    G.makeEdge(a - 1, b - 1);
  }
  return G;
}

int main() {
  Graph G = loadGraph();

  G.print();
  set<int>* cp = G.dfs(0, -1);
  for (int x : *cp) {
    cout << x << ' ';
  }
  cout << endl;
}
