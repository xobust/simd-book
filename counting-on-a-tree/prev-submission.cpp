#include <bits/stdc++.h>
using namespace std;

#define rep(i, from, to) for (int i = from; i < (to); ++i)
#define trav(a, x) for (auto& a : x)
#define all(x) x.begin(), x.end()
#define sz(x) (int)(x).size()
typedef long long ll;
typedef pair<int, int> pii;
typedef vector<int> vi;

enum { LARGE_LIM = 100 };

static vi vals;
static int M, LargeLim;

template <class T, int _SMALL>
struct Node {
	Node *left, *right;
	enum { SMALL = _SMALL };
	alignas(16) T vals[SMALL];
	int size;
	Node(int n) : left(0), right(0), size(n) {
		memset(vals, 0, sizeof vals);
	}
	void init() {
		if (size > SMALL) {
			int half = size & -size;
			if (half == size) half /= 2;
			left = new Node(size - half);
			right = new Node(half);
			left->init();
			right->init();
		}
	}
	Node* add(int ind) {
		if (ind < 0 || ind >= size) return this;
		Node* t = new Node(size);
		if (size <= SMALL) {
			rep(i,0,SMALL)
				t->vals[i] = vals[i];
			t->vals[ind]++;
		}
		else {
			t->left = left->add(ind);
			t->right = right->add(ind - left->size);
		}
		return t;
	}
};

typedef Node<int8_t, 256> SmallTree;
typedef Node<int, 16> LargeTree;

struct Tree {
	SmallTree* smallTree;
	LargeTree* largeTree;
	Tree() {}
	Tree(bool) {
		smallTree = new SmallTree(LargeLim);
		largeTree = new LargeTree(M - LargeLim);
		smallTree->init();
		largeTree->init();
	}
	Tree* add(int ind) {
		Tree* t = new Tree();
		t->smallTree = smallTree->add(ind);
		t->largeTree = largeTree->add(ind - LargeLim);
		return t;
	}
};

/*
map<pair<SmallTree*, SmallTree*>, int> mem;
int isect1(SmallTree* a, SmallTree* b) {
	const int SMALL = SmallTree::SMALL;
	assert(a->size == b->size);
	if (a->size <= a->SMALL) {
		int8_t *pa = a->vals, *pb = b->vals;
		int ret = 0;
		rep(i,0,SMALL) {
			ret += pa[i] * (int)pb[i];
		}
		return ret;
	}
	if (a->size <= 5*SMALL) {
		return isect1(a->left, b->left) + isect1(a->right, b->right);
	}
	int& out = mem[make_pair(a, b)];
	if (out) return out - 1;
	int ret = isect1(a->left, b->left) + isect1(a->right, b->right);
	out = ret + 1;
	return ret;
}

map<pair<LargeTree*, LargeTree*>, ll> mem2;
ll isect2(LargeTree* a, LargeTree* b) {
	const int SMALL = LargeTree::SMALL;
	assert(a->size == b->size);
	if (a->size <= SMALL) {
		int *pa = a->vals, *pb = b->vals;
		ll ret = 0;
		rep(i,0,SMALL) {
			ret += pa[i] * (ll)pb[i];
		}
		return ret;
	}
	if (a->size <= 5*SMALL) {
		return isect2(a->left, b->left) + isect2(a->right, b->right);
	}
	ll& out = mem2[make_pair(a, b)];
	if (out) return out - 1;
	ll ret = isect2(a->left, b->left) + isect2(a->right, b->right);
	out = ret + 1;
	return ret;
}

ll isect(Tree* a, Tree* b) {
	return isect1(a->smallTree, b->smallTree) + isect2(a->largeTree, b->largeTree);
}
*/

template<class T, int SMALL>
ll isect(Node<T, SMALL>** a, Node<T, SMALL>** b) {
	int size = a[0]->size;
	rep(i,0,4) assert(a[i]->size == size && b[i]->size == size);
	if (size <= SMALL) {
		ll ret = 0;
		rep(i,0,SMALL) {
			T av = (T)(a[0]->vals[i] + a[1]->vals[i] - a[2]->vals[i] - a[3]->vals[i]);
			T bv = (T)(b[0]->vals[i] + b[1]->vals[i] - b[2]->vals[i] - b[3]->vals[i]);
			ret += av * (ll)bv;
		}
		return ret;
	}
	else {
		ll ret = 0;
		Node<T, SMALL> *sa[4], *sb[4];
		rep(i,0,4) sa[i] = a[i]->left, sb[i] = b[i]->left;
		ret += isect(sa, sb);
		rep(i,0,4) sa[i] = a[i]->right, sb[i] = b[i]->right;
		ret += isect(sa, sb);
		return ret;
	}
}

ll isect(Tree** a, Tree** b) {
	SmallTree *sa[4], *sb[4];
	LargeTree *la[4], *lb[4];
	rep(i,0,4) sa[i] = a[i]->smallTree;
	rep(i,0,4) sb[i] = b[i]->smallTree;
	rep(i,0,4) la[i] = a[i]->largeTree;
	rep(i,0,4) lb[i] = b[i]->largeTree;
	return isect(sa, sb) + isect(la, lb);
}

typedef vector<pii> vpi;
typedef vector<vpi> graph;
const pii inf(1 << 29, -1);

template <class T>
struct RMQ {
	vector<vector<T>> jmp;

	RMQ(const vector<T>& V) {
		int N = sz(V), on = 1, depth = 1;
		while (on < sz(V)) on *= 2, depth++;
		jmp.assign(depth, V);
		rep(i,0,depth-1) rep(j,0,N)
			jmp[i+1][j] = min(jmp[i][j],
			jmp[i][min(N - 1, j + (1 << i))]);
	}

	T query(int a, int b) {
		if (b <= a) return inf;
		int dep = 31 - __builtin_clz(b - a);
		return min(jmp[dep][a], jmp[dep][b - (1 << dep)]);
	}
};

struct LCA {
	vi time, par, fdepth;
	vector<Tree*> tr;
	vector<ll> dist;
	RMQ<pii> rmq;

	LCA(graph& C, Tree* empty) :
		time(sz(C), -99), par(sz(C)), fdepth(sz(C)), tr(sz(C)), dist(sz(C)), rmq(dfs(C, empty)) {}

	vpi dfs(graph& C, Tree* empty) {
		vector<tuple<int, int, int, ll> > q(1);
		vpi ret;
		int T = 0, v, p, d; ll di;
		while (!q.empty()) {
			tie(v, p, d, di) = q.back();
			q.pop_back();
			if (d) ret.emplace_back(d, p);
			par[v] = p;
			fdepth[v] = (vals[v] != -1);
			if (v) fdepth[v] += fdepth[p];
			time[v] = T++;
			dist[v] = di;
			Tree* t = (v == 0 ? empty : tr[p]);
			tr[v] = vals[v] == -1 ? t : t->add(vals[v]);
			trav(e, C[v]) if (e.first != p)
				q.emplace_back(e.first, v, d+1, di + e.second);
		}
		return ret;
	}

	int query(int a, int b) {
		if (a == b) return a;
		a = time[a], b = time[b];
		return rmq.query(min(a, b), max(a, b)).second;
	}
	ll distance(int a, int b) {
		int lca = query(a, b);
		return dist[a] + dist[b] - 2 * dist[lca];
	}
};

int main() {
	cin.sync_with_stdio(false);
	cin.tie(0);
	cin.exceptions(cin.failbit);
	int N, Q;
	cin >> N >> Q;

	vals.resize(N);
	map<int, int> ren;
	rep(i,0,N) {
		cin >> vals[i];
		ren[vals[i]]++;
	}

	M = 0;
	LargeLim = -1;
	vector<pii> rens;
	rens.reserve(ren.size());
	trav(pa, ren) {
		rens.emplace_back(pa.second, pa.first);
		pa.second = -1;
	}
	sort(all(rens));
	trav(pa, rens) {
		if (pa.first > 1) {
			ren[pa.second] = M++;
			if (pa.first > LARGE_LIM && LargeLim == -1)
				LargeLim = M-1;
		}
	}
	rep(i,0,N) vals[i] = ren[vals[i]];
	if (LargeLim == -1) LargeLim = M;

	vector<vpi> ed(N);
	rep(i,0,N-1) {
		int u, v;
		cin >> u >> v;
		--u, --v;
		ed[u].emplace_back(v, 1);
		ed[v].emplace_back(u, 1);
	}

	Tree* empty = new Tree(true);

	LCA lca(ed, empty);

	vector<Tree*> tr1, tr2;

	rep(i,0,Q) {
		int w,x,y,z;
		cin >> w >> x >> y >> z;
		--w,--x,--y,--z;

		vector<pii> nodes1, nodes2;
		auto getn = [&](int a, int b, vector<pii>& out) {
			int c = lca.query(a, b);
			out.emplace_back(a, 1);
			out.emplace_back(b, 1);
			out.emplace_back(c, -1);
			if (c != 0)
				out.emplace_back(lca.par[c], -1);
		};
		getn(w, x, nodes1);
		getn(y, z, nodes2);

		ll res = 0;
		trav(a, nodes1) trav(b, nodes2) {
			ll v = lca.fdepth[lca.query(a.first, b.first)];
			res -= v * a.second * b.second;
		}

		tr1.clear();
		tr2.clear();
		trav(x, nodes1) tr1.push_back(lca.tr[x.first]);
		trav(x, nodes2) tr2.push_back(lca.tr[x.first]);
		if (tr1.size() < 4) tr1.push_back(empty);
		if (tr2.size() < 4) tr2.push_back(empty);
		res += isect(tr1.data(), tr2.data());
		/*
		rep(i,0,4) rep(j,0,4) {
			ll v = isect(tr1[i], tr2[j]);
			if (i & 2) v *= -1;
			if (j & 2) v *= -1;
			res += v;
		}
		*/
		cout << res << endl;
	}
	_Exit(0);
}
