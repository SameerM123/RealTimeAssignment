import { useEffect, useMemo } from 'react'
import { rebuildSpacedPlan } from '../lib/rules'
import { usePlannerStore, useStudentStore } from '../state/store'

export function PlannerPage() {
  const intensity = usePlannerStore((s) => s.intensity)
  const setIntensity = usePlannerStore((s) => s.setIntensity)
  const plan = usePlannerStore((s) => s.plan)
  const setPlan = usePlannerStore((s) => (entries: any[]) => (s.plan = entries))
  const mastery = useStudentStore((s) => s.masteryByConcept)

  useEffect(() => {
    const entries = rebuildSpacedPlan(mastery, intensity)
    setPlan(entries)
  }, [intensity, mastery, setPlan])

  const grouped = useMemo(() => {
    const map = new Map<string, number>()
    plan.forEach((e) => map.set(e.date, (map.get(e.date) ?? 0) + 1))
    return Array.from(map.entries()).sort(([a], [b]) => (a < b ? -1 : 1))
  }, [plan])

  return (
    <div className="space-y-4">
      <div className="card" role="group" aria-label="Intensity">
        <div className="text-sm text-muted">Intensity</div>
        <div className="mt-2 flex gap-2">
          {(['light', 'standard', 'intense'] as const).map((i) => (
            <button key={i} className={`btn ${intensity === i ? 'ring-2 ring-brand' : ''}`} onClick={() => setIntensity(i)}>
              {i[0].toUpperCase() + i.slice(1)}
            </button>
          ))}
        </div>
      </div>
      <div className="card">
        <div className="text-sm text-muted">Plan (next 14 days)</div>
        <div className="mt-3 grid grid-cols-7 gap-2">
          {grouped.map(([date, count]) => (
            <div key={date} className="rounded-md border border-border p-2 text-center text-sm">
              <div className="text-muted">{date.slice(5)}</div>
              <div className="mt-1 text-lg font-semibold">{count}</div>
            </div>
          ))}
        </div>
      </div>
      <div className="card">
        <div className="text-sm text-muted">Plan List</div>
        <ul className="mt-2 space-y-2">
          {plan.map((e, idx) => (
            <li key={idx} className="flex items-center justify-between rounded-md border border-border p-2 text-sm">
              <span className="text-muted">{e.date}</span>
              <span>{e.concept}</span>
              <span className="text-muted">{e.reason}</span>
            </li>
          ))}
        </ul>
      </div>
    </div>
  )
}


