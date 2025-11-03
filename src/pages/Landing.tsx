import { Link } from 'react-router-dom'

export function LandingPage() {
  return (
    <div className="space-y-8">
      <section className="card">
        <h1 className="mb-2 text-2xl font-semibold">Se7enAcademy — Intelligent Tutoring System</h1>
        <p className="text-muted">Front-end high-fidelity prototype for high-school science.</p>
        <div className="mt-4 flex flex-wrap gap-3">
          <Link className="btn" to="/student/session?mode=guided">Start Session</Link>
          <Link className="btn" to="/teacher">Open Teacher Dashboard</Link>
          <Link className="btn" to="/concepts">View Concept Map</Link>
        </div>
      </section>

      <section className="grid gap-4 md:grid-cols-2 lg:grid-cols-4">
        {[
          { t: 'Domain', s: 'Concepts, items, misconceptions' },
          { t: 'Student', s: 'Mastery, history, spaced review' },
          { t: 'Tutoring', s: 'Deterministic rules & hints' },
          { t: 'UI', s: 'Explainability, accessibility, polish' },
        ].map((b) => (
          <div key={b.t} className="card">
            <div className="text-sm text-muted">{b.t}</div>
            <div className="mt-1 font-medium">{b.s}</div>
          </div>
        ))}
      </section>
    </div>
  )
}


