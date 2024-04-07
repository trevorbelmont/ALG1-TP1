#include <iostream>
#include <set>
#include <stack>
#include <utility>
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

stack<int> S;
vector<vector<int>> components;
int timeStamp;

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
// O TAD do Grafo
struct Vertex {
  int id;
  bool isCutpoint;
  set<int> adj;  // lista de adjacências
  int parent;
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
      v[i].parent = -1;
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
        cout << "  ~(" << min[i] << ")    "
             << "par=" << char(65 + v[i].parent) << endl;
      }
    } else if (alfabetic == false) {
      for (int i = 0; i < size; i++) {  // para cada vértice do grafo
        cout << '[' << v[i].id + 1 << "-" << open[i] << "]";
        for (int x : v[i].adj) {  // imprime a lista de adjacência com letras
          cout << ' ' << x + 1;
        }
        cout << "  ~(" << min[i] << ")"
             << "par=" << v[i].parent << endl;
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

        else if (v[u].isCutpoint == true) {  // if achou cutpoint coloca no component mas não roda dfs dele ainda
          component->insert(u);
          // clustered[u] == true;
        }
      }
      if (v[w].isCutpoint) {
        // ¬¬ !clustered[u] ??
        if (clustered[u] == false && v[u].isCutpoint) {
          dfsComponent(u, w, clustered, component);  // ¬¬¬¬¬¬¬¬¬¬ só add um novo lowpoint se ele estiver conectado com os anteriores
          // pode fazer uma lista e checar no final do if
          if (!isBiconnected(*component)) {
            component->erase(u);
            clustered[u] = false;  // paleativo
          }
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

  // Só pode ser chamada depois da primeira DFS e retorna o component do vértice de origem, w.
  // Precisa retornar um ponteiro (?) pois, no geral, ela recebe o set<int> que retornará como entrada // pega casos brabos mas é instável
  set<int>* dfsComponent2(int w, int parent, bool clustered[], set<int>* component = nullptr, set<int>* lows = nullptr) {
    clustered[w] = true;
    if (component == nullptr) component = new set<int>;
    component->insert(w);

    if (lows == nullptr) {  // ou parent == -1
      lows = new set<int>;
    }

    for (int u : v[w].adj) {
      if (u == parent) {
        continue;
      }

      if (component->count(u)) {
        continue;
      }

      // se atualmente não está num cutpoint
      if (!v[w].isCutpoint) {
        if (clustered[u] == false && v[u].isCutpoint == false) {  // se o vizinho do w (atual) não foi visitado e não é cutpoint
          if (lows->empty()) lows->insert(min[w]);
          lows->insert(min[u]);

          dfsComponent2(u, w, clustered, component, lows);
        }

        else if (v[u].isCutpoint == true) {  // se o vizinho é cutpoint
          int nLows = lows->size();
          switch (nLows) {
            case 0:
              // lows->insert(min[u]); // não adiciona ainda. só adiciona se voltar (lows.size >= 1)
              lows->insert(min[w]);
              lows->insert(min[u]);
              component->insert(u);
              // dfsComponent(u, w, clustered, component, lows);
              break;
            case 1:
              lows->insert(min[u]);
              lows->insert(min[w]);
              dfsComponent2(u, w, clustered, component, lows);
            case 2:
              if (lows->count(min[u])) {  // só continua dfs no vértice se tiver low previsto (lista de lows já é igual a 2)
                dfsComponent2(u, w, clustered, component, lows);
              }

            default:
              if (VERBOSE) {
                cout << "!lows overflow em :  " << w + 1 << " cheking " << u + 1 << "  min[u] = " << min[u] << endl;
                cout << "set : ";
                printSet(*component, ' ', ALFABETICAL, VERBOSE);
                cout << "size of lows: " << nLows << endl;
              }
              continue;
              // exit(1);
              break;
          }
        }
      }

      if (v[w].isCutpoint) {
        // ¬¬ !clustered[u] ??
        if (!v[u].isCutpoint) {  // de um cutpoint acha um não cutpoint não explorado
          if (lows != nullptr && lows->count(min[u])) {
            dfsComponent2(u, w, clustered, component, lows);  // vai pra um vértice interno se o low foi previsto (independente do tamnho da lista lows)
          } else if (lows->size() <= 1) {
            if (VERBOSE) {
              lows->insert(min[u]);
              lows->insert(min[w]);
              cout << w + 1 << '-' << u + 1 << "  Lows: ";
              printSet(*lows, ' ');
            }
            dfsComponent2(u, w, clustered, component, lows);
          }
        }
        if (v[u].isCutpoint) {        // de um cutpoint acha outro cutpoint
          if (lows->count(min[u])) {  // faz parte da component (pela numerologia) (será que em todos os casos??
            dfsComponent2(u, w, clustered, component, lows);
          } else if (lows->size() < 2) {
            lows->insert(min[u]);
            dfsComponent2(u, w, clustered, component, lows);
          }
        }
      }
    }
    /* cout << " comp: ";
    printSet(*component, ' ');
    // Checar se o cluster é inválido (pelo número de lows) */
    return component;
  }
  /*

  s


  s



  */
  int tarjan(int u) {
    open[u] = min[u] = ++timeStamp;
    S.push(u);

    for (int i : v[u].adj) {
      if (open[i] == 0) {
        v[i].parent = u;
        tarjan(i);
        min[u] = minimum(min[u], min[i]);
        if (min[i] >= open[u]) {
          vector<int> component;
          int w;
          do {
            w = S.top();
            S.pop();
            component.push_back(w);
          } while (w != i);
          component.push_back(u);
          components.push_back(component);
        }
      } else if (i != v[u].parent) {
        min[u] = minimum(min[u], open[i]);
      }
    }
    return components.size();
  }

  int minimum(int a, int b) {
    if (a < b) {
      return a;
    } else {
      return b;
    }
  }

  bool isBiconnected(set<int> compCandidate) {
    if (compCandidate.size() <= 1) return false;
    if (compCandidate.size() == 2) {
      if (VERBOSE) {
        cout << "     Set de tamanho 2 retornando true. Set -> ";
        printSet(compCandidate, '*', ALFABETICAL, VERBOSE);
      }
      return true;
    }

    // Para candidatos a cluster com size >=3 checamos se todos os vértices tem grau interno de pelo menos 2.
    if (compCandidate.size() >= 3) {  // o caso que mais será utilizado
      for (int i : compCandidate) {
        int grau = 0;
        for (int j : compCandidate) {
          if (v[i].adj.count(j)) {
            grau++;
          }
        }
        if (grau < 2) {
          if (VERBOSE) {
            string qual = "";
            (ALFABETICAL) ? qual.append(1, char(65 + i)) : qual += to_string(i + 1);
            cout << "Não é uma component biconexa   (erro em    v[" << qual << "]   ): -> ";
            printSet(compCandidate, ' ', ALFABETICAL, VERBOSE);
          }
          return false;
        }
        // qualquer vértice do candidato que não não tiver no mínimo grau 2 torna a canditada não biconexa entre si.
      }
    }
    return true;  // passou por todas os vértices e encontoru biconectividade em todos
  }

  set<int> getComponent(int x, bool* clusteredExplored) {
    set<int>* clusterComponent = new set<int>;

    clusterComponent = dfsComponent(x, -1, clusteredExplored, clusterComponent);

    if (clusterComponent->size() <= 1) {  // "Links Isolados não fazem parte de nenhum cluster"
      set<int> empty;
      return empty;
    }
    // caso de retornar da dfsComponent1 com ver. interno "preso" entre dois cutpoints
    else if (clusterComponent->size() == 3) {
      if (!isBiconnected(*clusterComponent)) {  // CHECA BICONECTIVIDADE.Se não for, apaga
        clusterComponent->clear();
      }
    }
    set<int> comp = *(clusterComponent);
    return comp;
  }

  // Retorna as Componenents biconexas do grafo
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

          // apaga arestas de clusters válidos já prontos:
          for (int x : temp) {
            for (int y : temp) {
              if (x == y) continue;
              if (v[y].adj.count(x)) {
                v[x].adj.erase(y);
                v[y].adj.erase(x);
                if (VERBOSE) cout << "deleting edge (" << x << "-" << y << ")" << endl;
              }
            }
          }

          allClusters.insert(temp);
        }
      }
    }
    // Identificaremos os clusters que são formados apenas por cutpoints (bordas)

    for (int x : cutPoints) {
      for (int x : cutPoints) {
        clusteredExplored[x] = false;
      }

      set<int> temp = getComponent(x, clusteredExplored);
      if (temp.size() > 1) {
        if (true) {  // if (!isSubSet(temp, allClusters)) {  // ¬¬ muito pesadot
          if (VERBOSE) {
            cout << '~';
            (ALFABETICAL) ? cout << char(x + 65) : cout << (x + 1);
            cout << ": ";
            printSet(temp, ' ', ALFABETICAL, VERBOSE);
          }
          allClusters.insert(temp);
        } else if (VERBOSE) {
          cout << "discarded " << x << ": ";
          printSet(temp, ' ', ALFABETICAL, VERBOSE);
        }
      }
      // apaga arestas de clusters válidos já prontos:
      for (int x : temp) {
        for (int y : temp) {
          if (x == y) continue;
          if (v[y].adj.count(x)) {
            v[x].adj.erase(y);
            v[y].adj.erase(x);
            if (VERBOSE) cout << "deleting edge (" << x << "-" << y << ")" << endl;
          }
        }
      }
    }

    if (VERBOSE) {
      cout << endl
           << " OS CASOS BRABOS (CLUSTER MISTOS, COM MAIS DE DOIS CUTPOINTS E VÉRTICES INTERNOS) :" << endl
           << endl;

      for (int i = 0; i < size; i++) {
        cout << "v[" << i + 1 << "] = " << clusteredExplored[i] << endl;
      }
    }

    for (int x : cutPoints) {
      clusteredExplored[x] = false;
    }

    if (VERBOSE) {
      cout << endl;
      cout << "Grafo Depois das dfsComponent1 e antes da dfsComponent2 (pra pegar os casos brabos)" << endl;
      print();
      cout << endl;
    }

    if (VERBOSE) {
      cout << "Começa checar os brabos a partir dos cutpoints" << endl;
      cout << endl;
    }

    for (int i : cutPoints) {
      set<int>* brabos = new set<int>;
      if (v[i].adj.size() > 0) {
        dfsComponent2(i, -1, clusteredExplored, brabos);
        int valido = isBiconnected(*brabos);

        if (VERBOSE) {
          cout << "Brabo [" << i + 1 << "] (biconnected=" << valido << "): ";
          if (brabos != nullptr) printSet(*brabos, ' ', ALFABETICAL, VERBOSE);
        }
        if (valido) {
          for (int x : *brabos) {
            for (int y : *brabos) {
              if (x == y) continue;
              if (v[y].adj.count(x)) {
                v[x].adj.erase(y);
                v[y].adj.erase(x);
                if (VERBOSE) cout << "deleting edge (" << x << "-" << y << ")" << endl;
              }
            }
          }
          allClusters.insert(*brabos);
        }
      }
    }

    return allClusters;
  }

  set<int>*
  dfs(int w, int parent, int& t, set<int>* cutSet = nullptr) {
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
pair<int, string> ForestDump(set<set<int>> allClusters, set<int> cutpoints, int sizeOfGraph) {
  if (VERBOSE) {
    cout << endl;
    cout << "ForestDupster: " << endl;
    cout << "(V E) = ";
  }
  string foresting = "";
  int nEdges = 0;

  // int vertexCount, edgeCount;  // número de vértices e arestas do grafo condensado (Floresta Cluster-Bordas)
  // vertexCount = (allClusters.size() + cutpoints.size());
  // cout << vertexCount << ' ' << edgeCount << endl;

  if (!cutpoints.empty()) {  // se o grafo possui cutpoints (mais que um cluster)
    for (int cp : cutpoints) {
      int clustId = 0;
      for (set<int> cluster : allClusters) {
        if (cluster.find(cp) != cluster.end()) {  // se cp pertence ao cluster
          nEdges++;
          if (ALFABETICAL) {
            string s = "";
            s.append(1, char(cp + 65));
            // cout << c << s;

            foresting += s + " " + char(65 + (clustId + sizeOfGraph)) + '\n';

          } else {
            foresting += to_string(cp + 1) + " " + to_string(clustId + sizeOfGraph + 1) + '\n';
          }
        }
        clustId++;
      }
    }
  }
  pair<int, string> res(nEdges, foresting);
  return res;
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

  for (int i = 0; i < G.size; i++) {
    G.open[i] = 0;
  }
  set<set<int>> clusters;  //= G.getComponents(*cp);
  for (int i = 0; i < G.size; i++) {
    if (G.open[i] == 0)
      int tarjanComp = G.tarjan(i);
  }

  for (vector<int> vec : components) {
    set<int> aux;
    for (int i : vec) {
      aux.insert(i);
    }
    clusters.insert(aux);
  }

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

  pair<int, string> forest = ForestDump(clusters, *cp, G.size);
  int nForest = clusters.size() + cp->size();
  cout << nForest << ' ' << forest.first << endl;
  cout << forest.second;
}