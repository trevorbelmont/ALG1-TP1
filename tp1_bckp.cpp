#include <iostream>
#include <set>
#include <vector>
// ¬¬ por enquanto assumimos que 'A' = 0ª letra do alfabeto (sem offsets)
// ¬¬ Relações de low dos internos tendem a ser igual ao tempo de entrada do cluster (ou todos os lows são iguais - no caso do cluster que contem o root)
// ¬¬ Avaliando clusters formados por cutpoints, apenas são cutpoints-de-cutpoints vertex(cutpoints) que não são vizinhos de vértices internos (?)
// !! Checando cutpoint-clusters: dois cutpoints só formam um cluster se eles tiverem diferentes lowpoints?
//   vizinhos com low-points iguais deveriam ser aceitos??

// !! ¬¬ ainda tem erro no caso scorpion (clustofbordas)

/*
 http://graphonline.ru/pt/?graph=kIpBhNLISfetMSFC      ( scorpion )
*/
using namespace std;

bool ALFABETICAL = false;
bool VERBOSE = false;
int ROOT1STDFS = 0;

void printSet(set<int> s, char separator, bool alfabetic = false, bool verbose = false) {
  if (verbose) cout << '[' << s.size() << "]:" << separator;
  if (alfabetic) {
    for (int i : s) {
      cout << char(i + 65) << separator;
    }
    if (separator != '\n') cout << endl;  // evita uma quebra de linha a mais se JÁ ESTIVER SEPARANDO POR QUEBRAS DE LINHA
  } else {
    for (int i : s) {
      cout << (i + 1) << separator;
    }
    if (separator != '\n') cout << endl;  // evita uma quebra de linha a mais se JÁ ESTIVER SEPARANDO POR QUEBRAS DE LINHA
  }
}

bool isSubSet(set<int> s, set<set<int>> sets) {
  bool subSet = false;
  for (set super : sets) {
    int intersect = 0;
    if (super.size() < 2) {
      continue;
    }
    for (int x : s) {
      if (super.count(x)) {  // ¬¬
        intersect++;
      }
      if (intersect >= 2) {
        return true;
      }
    }
  }
  return subSet;
}

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

  // Imprime a lista de adjacências com letras ou números
  void print(bool alfabetic = false) {
    cout << "Listas de Adjacência:" << endl;

    if (alfabetic) {
      for (int i = 0; i < size; i++) {  // para cada vértice do grafo
        cout << '[' << char(65 + v[i].id) << "-" << open[i] << "]";
        for (int x : v[i].adj) {  // imprime a lista de adjacência com letras
          cout << ' ' << char(65 + x);
        }
        cout << "  ~(" << min[i] << ")" << endl;
      }
    } else if (alfabetic == false) {
      for (int i = 0; i < size; i++) {  // para cada vértice do grafo
        cout << '[' << v[i].id + 1 << "-" << open[i] << "]";
        for (int x : v[i].adj) {  // imprime a lista de adjacência com letras
          cout << ' ' << x + 1;
        }
        cout << "  ~(" << min[i] << ")" << endl;
      }
    }
  }

  // Só pode ser chamada depois da primeira DFS e retorna o component do vértice de origem, w.
  // Precisa retornar um ponteiro (?) pois, no geral, ela recebe o set<int> que retornará como entrada
  set<int>* dfsComponent(int w, int parent, bool clustered[], set<int>* component = nullptr) {
    clustered[w] = true;
    component->insert(w);

    for (int u : v[w].adj) {
      if (u == parent) {
        continue;
      }
      if (!v[w].isCutpoint) {
        if (clustered[u] == false && v[u].isCutpoint == false) {
          dfsComponent(u, w, clustered, component);
        }

        else if (v[u].isCutpoint == true) {
          component->insert(u);
          clustered[u] == true;
          continue;
        }
      }
      if (v[w].isCutpoint) {
        // ¬¬ !clustered[u] ??
        if (clustered[u] == false && v[u].isCutpoint) {
          dfsComponent(u, w, clustered, component);
        }

        // Uma vez em um cutPoint não visita um não-cutpoint (começou em cutPoint)
        if (v[u].isCutpoint == false) {
          if (VERBOSE) cout << '\t' << "edge (" << char(w + 65) << "," << char(u + 65) << ") was ignored" << endl;
          continue;  // checa próximo vizinho
        }
      }
    }
    return component;
  }

  set<int> getComponent(int x, bool* clusteredExplored) {
    set<int>* clusterComponent = new set<int>;

    clusterComponent = dfsComponent(x, -1, clusteredExplored, clusterComponent);

    if (clusterComponent->size() <= 1) {  // "Links Isolados não fazem parte de nenhum cluster"
      set<int> empty;
      return empty;
    }
    set<int> comp = *(clusterComponent);
    return comp;
  }

  // Retorna
  set<set<int>> getComponents(set<int> cutPoints) {
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
    set<set<int>> allClusters;

    // primeiro identifica os clusters a partir dos vértices que não são cutpoints.
    // Os vértices visitados na dfsComponent são marcados como explorados
    for (int x : notCutPoint) {
      // evita que tente identificar o mesmo cluster mais que uma vez
      if (clusteredExplored[x] == false) {  // portanto, só roda dfsComponent em não cutPoints ainda não explorados.
        set<int> temp = getComponent(x, clusteredExplored);
        if (temp.size() > 1) {  // "vértices isoladoss não são clusters"
          if (VERBOSE) {
            cout << char(x + 65) << ": ";
            printSet(temp, ' ', ALFABETICAL, VERBOSE);
          }

          for (int x : temp) {
            if (v[x].isCutpoint) {
              for (int y : temp) {
                if (x == y) continue;
                if (v[y].isCutpoint) {
                  v[x].adj.erase(y);
                  v[y].adj.erase(x);
                  if (VERBOSE) cout << "deleting edge (" << x << "-" << y << ")" << endl;
                }
              }
            }
          }
          allClusters.insert(temp);
        }
      }
    }

    // Agora que identificamos todos clusters que possuem no mínimo um não-cutpoint (e marcamos os vértices visistados)
    // Identificaremos os clusters que são formados apenas por cutpoints (bordas)
    for (int x : cutPoints) {
      clusteredExplored[x] = false;
    }

    for (int x : cutPoints) {
      set<int> temp = getComponent(x, clusteredExplored);
      if (temp.size() > 1) {
        if (true) {  // if (!isSubSet(temp, allClusters)) {  // ¬¬ muito pesadot
          if (VERBOSE) {
            cout << '~' << char(x + 65) << ": ";
            printSet(temp, ' ', ALFABETICAL, VERBOSE);
          }
          allClusters.insert(temp);
        } else if (VERBOSE) {
          cout << "discarded " << x << ": ";
          printSet(temp, ' ', ALFABETICAL, VERBOSE);
        }
      }
    }
    return allClusters;
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

        // se o min (vertex de retorno mínimo) do vizinho u é menor que o mínimo do vertex w(atual), w é cutpoint.
        if (min[u] >= open[w] && parent != -1) {
          if (VERBOSE) cout << " ============== IS c u t p o i n t (" << w << ") ============" << endl;
          cutPoints->insert(w);
          v[w].isCutpoint = true;
        }
        children++;
      }
    }
    if (parent == -1 && children > 1) {  // se é o vértice raiz da DFS e tem mais que um filho
      cutPoints->insert(w);
      v[w].isCutpoint = true;
      if (VERBOSE) cout << " ============== IS c u t p o i n t (" << w << ") ============" << endl;
    }
    t++;
    return cutPoints;
  }
};

Graph loadGraph() {
  int N, M;

  cin >> N >> M;

  // se o limite o número de vértices for maior que o número de letras do alfabeto (maiúsculo) ¬
  // força o desligamento do verbose
  if (VERBOSE && N > 26) {
    VERBOSE = false;  // não existem tantas letras maiúscualas assim, parça...
  }

  Graph G(N);

  int a, b;  // fora do loop?? ¬
  for (int i = 0; i < M; i++) {
    cin >> a >> b;

    G.makeEdge(a - 1, b - 1);
  }
  return G;
}

// Um gerador de Floresta Clusters-Bordas que retorna um Grafo
void ForestDump(set<set<int>> allClusters, set<int> cutpoints, int sizeOfGraph) {
  if (VERBOSE) {
    cout << endl;
    cout << "ForestDupster: " << endl;
    cout << "(V E) = ";
  }

  int vertexCount, edgeCount;  // número de vértices e arestas do grafo condensado (Floresta Cluster-Bordas)
  vertexCount = (allClusters.size() + cutpoints.size());
  edgeCount = vertexCount - 1;
  cout << vertexCount << ' ' << edgeCount << endl;

  if (!cutpoints.empty()) {  // se o grafo possui cutpoints (mais que um cluster)
    for (int cp : cutpoints) {
      int clustId = 0;
      for (set<int> cluster : allClusters) {
        if (cluster.find(cp) != cluster.end()) {  // se cp pertence ao cluster
          if (ALFABETICAL) {
            cout << char(cp + 65) << " " << char(65 + (clustId + sizeOfGraph)) << endl;  // Grafo com letras
          } else {
            cout << (cp + 1) << " " << ((clustId + sizeOfGraph + 1)) << endl;
          }
        }
        clustId++;
      }
    }
  }
}

int main(int charc, char** charv) {
  if (charc > 1) {
    for (int i = 1; i < charc; i++) {
      if (charv[i][0] == '-') {
        switch (charv[i][1]) {
          case 'a':
            ALFABETICAL = true;
            break;
          case 'v':
            VERBOSE = true;
            break;
          case 'r':
            ROOT1STDFS = stoi(charv[i + 1]);
            if (VERBOSE) {
              cout << "ROOT of 1st DFS has been changed to vertex [";
              (ALFABETICAL) ? cout << char(65 + ROOT1STDFS) : cout << ROOT1STDFS + 1;
              cout << "]." << endl;
            }

          default:
            break;
        }
      }
    }
  }

  Graph G = loadGraph();

  int t = 0;

  // checa se a dretriz de vértice original da 1ª DFS é um vértice válido
  ROOT1STDFS = (ROOT1STDFS < G.size) ? ROOT1STDFS : 0;

  set<int>* cp = G.dfs(ROOT1STDFS, -1, t);

  if (VERBOSE) {
    G.print(ALFABETICAL);
    cout << endl;
    cout << "Listing the " << (*cp).size() << " CutPoints: ";
    printSet(*cp, ' ', ALFABETICAL, true);
  } else {
    cout << cp->size() << endl;
    printSet(*cp, '\n', ALFABETICAL, false);
  }

  set<set<int>> clusters = G.getComponents(*cp);

  if (VERBOSE) {
    cout << endl;
    cout << "Listing the " << clusters.size() << " clusters:" << endl;
  } else {
    cout << clusters.size() << endl;
  }
  int id = 0;
  for (set<int> s : clusters) {
    if (ALFABETICAL) {
      cout << char((id + G.size) + 65) << ' ';
    } else {
      cout << (id + G.size + 1) << ' ';
    }

    if (VERBOSE) {
      cout << '[' << s.size() << "] ";
    } else {
      cout << s.size() << ' ';
    }
    printSet(s, ' ', ALFABETICAL);
    id++;
  }

  ForestDump(clusters, *cp, G.size);
}