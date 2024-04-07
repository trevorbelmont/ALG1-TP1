#include <stdio.h>
#include <string.h>

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
      printf("%d%c", (i + 1), separator);
      // cout << (i + 1) << separator;
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

  vector<int> explored;
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
    explored.assign(size, 0);
    open.assign(size, -1);
    min.assign(size, -1);
  }
  void reset() {
    for (int i = 0; i < size; i++) {
      v[i].id = i;
      v[i].isCutpoint = false;
      v[i].parent = -1;
    }
    // Inicializa todas as variáveis auxiliares
    explored.assign(size, 0);
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

  // Retorna um set contendo os cutpoints do grafo
  set<int> dfs(int w, int parent, int& t, set<int>& cutSet) {
    // Baseado no Algoritmo de consiguir cutpoints da site cp-algorithm

    explored[w] = true;
    open[w] = min[w] = t++;  // inicialmente min[w] = ao tempo de abertura (open) de w
    int children = 0;

    for (int u : v[w].adj) {
      if (u == parent) continue;                          // se a aresta volta pro pai, ignora.
      if (explored[u] == true) {                          // se o vizinho já foi visitado na DFS (aresta de retorno)
        min[w] = (min[w] <= open[u]) ? min[w] : open[u];  // min[w] = mínimo(min[w], open[u])
      }

      else {                                            // se o vizinho ainda não foi visitado
        dfs(u, w, t, cutSet);                           // Chama DFS do vizinho não explorado
        min[w] = (min[w] <= min[u]) ? min[w] : min[u];  // se min de w é menor que de seus descendentes na DFSTree permanece
                                                        // caso o contário o min w passa a ser o min de seu descendente

        // se o min (vertex de retorno mínimo) do vizinho u é menor que o mínimo do vertex w(atual), w é cutpoint.
        if (min[u] >= open[w] && parent != -1) {
          if (VERBOSE) cout << " ============== IS c u t p o i n t (" << w << ") ============" << endl;
          cutSet.insert(w);
          v[w].isCutpoint = true;
        }
        children++;
      }
    }
    if (parent == -1 && children > 1) {  // se é o vértice raiz da DFS e tem mais que um filho
      cutSet.insert(w);
      v[w].isCutpoint = true;
      if (VERBOSE) cout << " ============== IS c u t p o i n t (" << w << ") ============" << endl;
    }
    t++;
    return cutSet;
  }

  // Baseado no algoritmo de Robert Endre Tarjan como exposto em : https://iq.opengenus.org/biconnected-components/
  set<set<int>> tarjan(set<set<int>>& allClusters, int w, set<int>& cutpoints, stack<int>& pilha, int time = -1) {
    if (time == -1) {
      v[w].parent = -1;
    }

    explored[w] = 1;            // Colore de explorando (1)
    min[w] = open[w] = ++time;  // incrementa contador e depois assinala.
    pilha.push(w);
    int children_in_the_dfsTree = 0;  // contador de filhos na árvore da dfs (implementação do cp-algorithm)

    // Olha pra vizinhança do vértice atual, w.
    for (int u : v[w].adj) {
      // Checa se u ainda não foi explorado ou não se está explorando
      if (explored[u] == 0) {
        v[u].parent = w + 1;
        children_in_the_dfsTree++;
        tarjan(allClusters, u, cutpoints, pilha, time);
        min[w] = minimum(min[w], min[u]);

        if (min[u] >= open[w]) {  // Encontra Candidato a cutPoint

          // Garante que é Cutpoint (porém não é a raiz)
          // o caso da raiz é tratado no fechamento do vértice)
          if (v[w].parent > -1) {
            cutpoints.insert(w);
            v[w].isCutpoint = true;
          }
          set<int> cluster;
          int vertex;

          // Antes de sair do vértice adicona os vértices visitados ao component
          do {
            vertex = pilha.top();
            pilha.pop();

            cluster.insert(vertex);
          } while (vertex != u);

          cluster.insert(w);
          allClusters.insert(cluster);
        }
      } else if (u != v[w].parent) {
        min[w] = minimum(min[w], open[u]);
      }
    }
    if (v[w].parent == -1 && children_in_the_dfsTree >= 2) {
      v[w].isCutpoint = true;
      cutpoints.insert(w);
    }

    explored[w] = 2;  // Colore de já explorado (2) quando fecha o vértice
    return allClusters;
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
};

Graph loadGraph() {
  int N, M;

  // cin >> N >> M;
  scanf("%d %d", &N, &M);

  // se o limite o número de vértices for maior que o número de letras do alfabeto (maiúsculo) ¬
  // força o desligamento do verbose
  if (VERBOSE && N > 26) {
    VERBOSE = false;  // não existem tantas letras maiúscualas assim, parça...
  }

  Graph G(N);

  int a, b;  // fora do loop?? ¬
  for (int i = 0; i < M; i++) {
    scanf("%d %d", &a, &b);
    // cin >> a >> b;

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

  // checa se a dretriz de vértice original da 1ª DFS é um vértice válido
  ROOT1STDFS = (ROOT1STDFS < G.size) ? ROOT1STDFS : 0;

  set<set<int>> clusters;
  set<int> tarjanCps;
  int t = -1;

  // Se uma raíz não trivial (lexicográfica) foi selecionada, processa ela primeiro.
  if (ROOT1STDFS != 0) {
    stack<int> pilhaDeComponents;  // vazia
    clusters = G.tarjan(clusters, ROOT1STDFS, tarjanCps, pilhaDeComponents, t);
  }

  // Faz DFS nos outros vértices em ordem lexicográfica
  for (int i = -1; i < G.size; i++) {
    if (G.open[i] == -1) {
      stack<int> pilhaDeComponents;  // vazia
      clusters = G.tarjan(clusters, i, tarjanCps, pilhaDeComponents, t);
    }
  }

  if (VERBOSE) {
    G.print(ALFABETICAL);
    cout << endl;
    cout << "Listing the " << tarjanCps.size() << " CutPoints: ";
    printSet(tarjanCps, ' ', ALFABETICAL, true);
  } else {
    t = 0;
    G.reset();
    set<int> cp;
    G.dfs(ROOT1STDFS, -1, t, cp);
    printf("%d\n",cp.size());
    //cout << cp.size() << endl;
    // printSet(tarjanCps, '\n', ALFABETICAL, false);
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
      printf("%d ", (id + G.size + 1));
      // cout << (id + G.size + 1) << ' ';
    }

    if (VERBOSE) {
      cout << '[' << s.size() << "] ";
    } else {
      printf("%d ", s.size());
      // cout << s.size() << ' ';
    }
    printSet(s, ' ', ALFABETICAL);
    id++;
  }

  pair<int, string> forest = ForestDump(clusters, tarjanCps, G.size);
  int nForest = clusters.size() + tarjanCps.size();

  printf("%d %d\n%s", nForest, forest.first, (forest.second).c_str());
  /* cout << nForest << ' ' << forest.first << endl;
  cout << forest.second;  */}
