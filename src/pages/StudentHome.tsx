import { Link } from 'react-router-dom'

export function StudentHomePage() {
  return (
    <div className="space-y-4">
      <div className="card">
        <div className="text-sm text-muted">Readiness</div>
        <div className="mt-1 font-medium">Photosynthesis Module</div>
        <Link className="btn mt-3" to="/student/session?mode=guided">Continue Practice</Link>
      </div>
      <div className="card">
        <div className="text-sm text-muted">Practice Mode</div>
        <div className="mt-1">Guided Practice vs Quick Quiz (coming soon)</div>
      </div>
      <div className="card">
        <div className="text-sm text-muted">Plan Preview</div>
        <Link className="btn mt-3" to="/student/plan">Open Planner</Link>
      </div>
    </div>
  )
}


