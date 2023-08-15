class TreeIncrementalSolver:
  def __init__(self, Solver, max_solvers: int):
    self.Solver = Solver
    self.max_solvers = max_solvers
    self.recently_used = Queue()
    self.recycled_solvers = []

  def reuseOrCreateZ3(self, query):
    solver_from_scratch_cost = len(query)
    min_cost, min_solver = solver_from_scratch_cost, None
    for solver in self.recycled_solvers:
      delta = distance(solver.query, query)
      if delta < min_cost:
        min_cost, min_solver = delta, solver
    if min_solver is None:
      return self.Solver()
    return min_solver

  def find_suitable_solver(self, query):
    for solver in self.recently_used:
      if solver.query is subsetOf(query):
        self.recently_used.remove(solver)
        return solver
    if len(self.recently_used) < self.max_solvers:
      return self.reuseOrCreateZ3(query)
    return self.recently_used.pop_back()

  def check_sat(self, query):
    solver = self.find_suitable_solver(query)
    push(solver, query)
    self.recently_used.push_front(solver)
    solver.check_sat()

  def recycle_solver(self, id):
    raise NotImplementedError()
