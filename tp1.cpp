#include <iostream>
#include <set>
#include <vector>
// ¬¬ por enquanto assumimos que 'A' = 0ª letra do alfabeto (sem offsets)
using namespace std;

// O TAD do Grafo
struct Vertex {
  int id;
  bool isCutpoint;
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
      v[i].isCutpoint = false;
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
    cout << "Lista de Adjacência:" << endl;
    for (int i = 0; i < size; i++) {
      cout << '[' << char(65 + v[i].id) << "-" << open[i] << "]";
      /*      FORMA COMPLETA
      for (set<int>::iterator it = v[i].adj.begin(); it != v[i].adj.end(); it++) {
        cout << ' ' << *it;
      }
      */
      //       FORMA BONITA

      for (int x : v[i].adj) {
        cout << ' ' << char(65 + x);
      }
      cout << "  ~(" << min[i] << ")" << endl;
    }
    /* cout << "Explorado , Abertura, mínimo" << endl;
    for (int i = 0; i < size; i++) {
      cout << "~[" << i << ']' << ' ' << explored[i] << ' ' << open[i] << ' ' << min[i] << endl;
    } */
  }

  // Só pode ser chamada depois da primeira DFS
  set<int>* dfsComponent(int w, bool clustered[], set<int>* component = nullptr) {
    clustered[w] = true;
    component->insert(w);

    for (int u : v[w].adj) {
      if (clustered[u] == false && v[u].isCutpoint == false) {
        dfsComponent(u, clustered, component);
      } else if (v[u].isCutpoint == true) {
        component->insert(u);
        clustered[u] == true;
        continue;
      }
      if (v[w].isCutpoint) {
        return component;
      }
    }
    return component;
  }

  void getComponent(int x, bool* clusteredExplored) {
    set<int>* clusterComponent = new set<int>;

    clusterComponent = dfsComponent(x, clusteredExplored, clusterComponent);

    if (clusterComponent->size() <= 1) {  // "Links Isolados não fazem parte de nenhum cluster"
      return;
    }

    cout << "components (" << char(x + 65) << "): ";
    for (int x : *clusterComponent) {
      cout << char(x + 65) << " ";
    }
    cout << endl;
  }
  void getComponents(set<int> cutPoints) {
    bool clusteredExplored[size];
    set<int> notCutPoint;  // conjunto de veŕtices de G que não é borda/CutPoint/vértice de corte
    // prepara nova DFS (com ciência dos cutpoints)
    for (int i = 0; i < size; i++) {
      clusteredExplored[i] = false;

      if (v[i].isCutpoint == false) {  // Só adciona vértices que não são cutpoints
        notCutPoint.insert(i);
      }
    }
    //
    for (int x : notCutPoint) {
      if (clusteredExplored[x] == false)
        getComponent(x, clusteredExplored);
    }
    for (int x : cutPoints) {
      getComponent(x, clusteredExplored);
    }
  }

  set<int>* dfs(int w, int parent, int& t, set<int>* cutSet = nullptr) {
    set<int>* cutPoints;  // ||

    if (t != 0) {
      cutPoints = cutSet;
    } else {
      cutPoints = new set<int>;
    }

    explored[w] = true;
    open[w] = min[w] = t++;  // inicialmente min[w] = ao tempo de abertura (open) de w
    int children = 0;

    for (int u : v[w].adj) {
      if (u == parent) continue;                          // se a aresta volta pro pai, ignora.
      if (explored[u] == true) {                          // se o vizinho já foi visitado na DFS (aresta de retorno)
        min[w] = (min[w] <= open[u]) ? min[w] : open[u];  // min[w] = mínimo(min[w], open[u])
      }

      else {                                            // se o vizinho ainda não foi visitado
        dfs(u, w, t, cutPoints);                        // Chama DFS do vizinho não explorado
        min[w] = (min[w] <= min[u]) ? min[w] : min[u];  // se min de w é menor que de seus descendentes na DFSTree permanece
                                                        // caso o contário o min w passa a ser o min de seu descendente
        if (min[u] >= open[w] && parent != -1) {
          // cout << " ============== IS c u t p o i n t (" << w << ") ============" << endl;
          cutPoints->insert(w);
          v[w].isCutpoint = true;
        }
        children++;
      }
    }
    if (parent == -1 && children > 1) {  // se é o vértice raiz da DFS e tem mais que um filho
      cout << " ============== IS c u t p o i n t (" << v << ") ============" << endl;
    }
    t++;
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

  int t = 0;
  set<int>* cp = G.dfs(0, -1, t);

  G.print();
  cout << "CutPoints: ";
  for (int x : *cp) {
    cout << char(x + 65) << ' ';
  }
  cout << endl;
  cout << endl;

  G.getComponents(*cp);
}
