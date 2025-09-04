#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <cassert>

void parse_arg(int argc, char** argv, int* n, int *m) {
  if (argc != 3) {
    fprintf(stderr, "generator requires exactly 2 argument, but got %d\n", argc);
    exit(-1);
  }
  *n = std::stoi(argv[1]);
  *m = std::stoi(argv[2]);
  if (*n <= 0 || *m < 0 || *m >= *n) {
    fprintf(stderr, "invalid argument `%s %s` and, expected a positive integer", argv[1], argv[2]);
    exit(-1);
  }
}

struct Edge {
  Edge(std::size_t to, std::string cond) : to(to), cond(cond) {
  }
  std::size_t to;
  std::string cond;
};

struct Node {
  bool is_start = false;
  bool is_accept = false;
  std::vector<Edge> edges;

  void add_edge(int to, std::string cond) {
    edges.emplace_back(Edge(to, cond));
  }
  bool has_edge_to(std::size_t target) {
    for (auto edge : edges) {
      if (edge.to == target) {
        return true;
      }
    }
    return false;
  }
  Edge& edge_to(std::size_t idx) {
    assert(has_edge_to(idx));
    for (int i = 0; i < edges.size(); i++) {
      if (edges[i].to == idx) {
        return edges[i];
      }
    }
    throw "impossible";
  }
};

struct GNFA {
  void delete_node(std::size_t idx) {
    if (nodes[idx] != nullptr) delete nodes[idx];
    nodes[idx] = nullptr;

    for (auto& node : nodes) {
      if (node == nullptr) continue;
      for (int i = 0; i < node->edges.size(); i++) {
        if (node->edges[i].to == idx) {
          node->edges.erase(node->edges.begin() + i);
        }
      }
    }
  }

  std::vector<Node*> nodes;
};

void print_gnfa(GNFA* gnfa) {
  for (int i = 0; i < gnfa->nodes.size(); i++) {
    if (gnfa->nodes[i] == nullptr) continue;
    printf("%c", gnfa->nodes[i]->is_start ? '>' : gnfa->nodes[i]->is_accept ? '*' : ' ');
    printf("%2d: ", i);
    for (auto edge : gnfa->nodes[i]->edges) {
      printf("'%s' -> %lu | ", edge.cond.c_str(), edge.to);
    }
    printf("\n");
  }
}

GNFA* generate_gnfa(std::size_t n, std::size_t m) {
  auto* gnfa = new GNFA();
  for (int i = 0; i < n * (n + 1); i++) {
    gnfa->nodes.push_back(new Node);
  }
  gnfa->nodes[0]->is_start = true;
  for (int k = 0; k < n + 1; k++) {
    for (int i = 0; i < n; i++) {
      gnfa->nodes[k * n + i]->add_edge(k * n + (2 * i) % n, "0");
      gnfa->nodes[k * n + i]->add_edge(k * n + (2 * i + 1) % n, "1");
    }
  }
  for (int i = 0; i < n; i++) {
    if (gnfa->nodes[i]->has_edge_to((i + 1) * n)) {
      gnfa->nodes[i]->edge_to((i + 1) * n).cond += "|\\+";
    }
    else {
      gnfa->nodes[i]->add_edge((i + 1) * n, "\\+");
    }
  }
  for (int i = 0; i < n; i++) {
    gnfa->nodes[(i + 1) * n + (m - i + n) % n]->is_accept = true;
  }
  return gnfa;
}

struct NewEdge {
  NewEdge(std::size_t from, std::size_t to, std::string cond)
    : from(from), to(to), cond(cond)
  {
  }
  std::size_t from, to;
  std::string cond;
};

void equivalent_remove_node(GNFA* gnfa, std::size_t idx) {
  auto node = gnfa->nodes[idx];
  if (node == nullptr) return;

  std::vector<NewEdge> to_be_added;

  for (int i = 0; i < gnfa->nodes.size(); i++) {
    auto n1 = gnfa->nodes[i];
    if (n1 == nullptr) continue;
    for (int j = 0; j < gnfa->nodes.size(); j++) {
      auto n2 = gnfa->nodes[j];
      if (n2 == nullptr) continue;
      if (!n1->has_edge_to(idx) || !node->has_edge_to(j)) continue;

      std::string new_cond;

      auto cond1 = n1->edge_to(idx).cond;
      if (cond1.size() == 0) {
        /* epsilon */
      }
      else if (cond1 != "0" && cond1 != "1" && cond1 != "\\+") {
        new_cond.append('(' + cond1 + ')');
      }
      else {
        new_cond.append(cond1);
      }

      if (node->has_edge_to(idx)) {
        auto self_edge_cond = node->edge_to(idx).cond;
        if (self_edge_cond.size() == 0) {
          /* epsilon edge, ignore */
        }
        else if (self_edge_cond == "0" || self_edge_cond == "1" || self_edge_cond == "\\+") {
          new_cond.append(self_edge_cond + "*");
        }
        else if (*self_edge_cond.rbegin() == '*') {
          new_cond.append(self_edge_cond);
        }
        else {
          new_cond.append("(" + self_edge_cond + ")*");
        }
      }

      auto cond2 = node->edge_to(j).cond;
      if (cond2.size() == 0) {
        /* epsilon */
      }
      else if (cond2 != "0" && cond2 != "1" && cond2 != "\\+") {
        new_cond.append('(' + cond2 + ')');
      }
      else {
        new_cond.append(cond2);
      }

      if (n1->has_edge_to(j) && n1->edge_to(j).cond.size() > 0) {
        new_cond.append('|' + n1->edge_to(j).cond);
      }

      to_be_added.emplace_back(NewEdge(i, j, new_cond));
    }
  }

  gnfa->delete_node(idx);

  for (auto edge : to_be_added) {
    auto n1 = gnfa->nodes[edge.from], n2 = gnfa->nodes[edge.to];
    if (n1 == nullptr || n2 == nullptr) continue;
    if (n1->has_edge_to(edge.to)) {
      n1->edge_to(edge.to).cond = edge.cond;
    }
    else {
      n1->add_edge(edge.to, edge.cond);
    }
  }
}

void simplify_gnfa(GNFA* gnfa) {
  gnfa->nodes.push_back(new Node);
  gnfa->nodes.push_back(new Node);

  auto& start_node = gnfa->nodes[gnfa->nodes.size() - 2];
  auto& accept_node = gnfa->nodes[gnfa->nodes.size() - 1];
  start_node->is_start = true;
  accept_node->is_accept = true;

  for (int i = 0; i < gnfa->nodes.size() - 2; i++) {
    if (gnfa->nodes[i]->is_start) {
      gnfa->nodes[i]->is_start = false;
      start_node->add_edge(i, "");
    }
    if (gnfa->nodes[i]->is_accept) {
      gnfa->nodes[i]->is_accept = false;
      gnfa->nodes[i]->add_edge(gnfa->nodes.size() - 1, "");
    }
  }

  for (int i = 0; i < gnfa->nodes.size(); i++) {
    auto node = gnfa->nodes[i];
    if (node == nullptr || node->is_start || node->is_accept) continue;
    equivalent_remove_node(gnfa, i);
  }
}

std::string extract_regex(GNFA* gnfa) {
  int start_node = -1, accept_node = -1;
  for (int i = 0; i < gnfa->nodes.size(); i++) {
    auto& node = gnfa->nodes[i];
    if (node == nullptr) continue;
    if (node->is_start) {
      assert(start_node < 0);
      start_node = i;
    }
    if (node->is_accept) {
      assert(accept_node < 0);
      accept_node = i;
    }
  }
  assert(start_node >= 0 && accept_node >= 0);
  return gnfa->nodes[start_node]->edge_to(accept_node).cond;
}

int main(int argc, char** argv) {
  int n, m;
  parse_arg(argc, argv, &n, &m);
  auto gnfa = generate_gnfa(n, m);
  simplify_gnfa(gnfa);
  auto regex = extract_regex(gnfa);
  printf("%s\n", regex.c_str());
  return 0;
}