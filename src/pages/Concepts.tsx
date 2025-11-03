export function ConceptsPage() {
  return (
    <div className="grid gap-4 sm:grid-cols-2 lg:grid-cols-3">
      {[
        { id: 'photosynthesis_overview', name: 'Photosynthesis Overview' },
        { id: 'light_reactions', name: 'Light Reactions' },
        { id: 'calvin_cycle', name: 'Calvin Cycle' },
        { id: 'limiting_factors', name: 'Limiting Factors' },
      ].map((c) => (
        <div key={c.id} className="card">
          <div className="text-sm text-muted">Concept</div>
          <div className="mt-1 font-medium">{c.name}</div>
          <div className="mt-2 text-sm text-muted">Prereqs: —</div>
        </div>
      ))}
    </div>
  )
}


