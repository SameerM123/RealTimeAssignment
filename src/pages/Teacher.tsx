import { useMemo, useState } from 'react'
import { colorForMastery, concepts, roster } from '../lib/data'
import { buildCsv, downloadCsv } from '../lib/csv'

export function TeacherPage() {
  const [ack, setAck] = useState<Record<string, boolean>>({})

  const triage = useMemo(() => {
    return roster
      .map((r) => {
        const minConcept = Object.entries(r.mastery).sort((a, b) => a[1] - b[1])[0]
        return {
          student: r,
          worstConcept: minConcept?.[0] ?? 'unknown',
          value: minConcept?.[1] ?? 0,
          hasFlag: (r.flags ?? []).length > 0,
        }
      })
      .sort((a, b) => a.value - b.value)
  }, [])

  return (
    <div className="space-y-4">
      <div className="card" role="table" aria-label="Class Heatmap">
        <div className="text-sm text-muted">Class Heatmap</div>
        <div className="mt-3 overflow-auto">
          <table className="w-full text-sm">
            <thead className="text-left text-muted">
              <tr>
                <th className="px-2 py-1">Student</th>
                {concepts.map((c) => (
                  <th key={c.id} className="px-2 py-1">{c.name}</th>
                ))}
              </tr>
            </thead>
            <tbody>
              {roster.map((r) => (
                <tr key={r.student_id} className="border-t border-border">
                  <td className="px-2 py-1">{r.name}</td>
                  {concepts.map((c) => {
                    const v = (r.mastery as Record<string, number>)[c.id] ?? 0
                    const cls = colorForMastery(v)
                    return (
                      <td key={c.id} className="px-2 py-1">
                        <span className={`inline-block rounded px-2 py-1 ${cls}`}>{v}</span>
                      </td>
                    )
                  })}
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </div>
      <div className="card">
        <div className="text-sm text-muted">Triage List</div>
        <ul className="mt-2 space-y-2">
          {triage.map((t) => (
            <li key={t.student.student_id} className="flex items-center justify-between rounded-md border border-border p-2 text-sm">
              <span>
                <span className="font-medium">{t.student.name}</span>
                <span className="ml-2 text-muted">{t.worstConcept}</span>
              </span>
              <span className="flex items-center gap-2">
                {t.hasFlag && <span className="rounded bg-warn/20 px-2 py-1 text-warn">flag</span>}
                {!ack[t.student.student_id] ? (
                  <button className="btn" onClick={() => setAck({ ...ack, [t.student.student_id]: true })}>Acknowledge</button>
                ) : (
                  <span className="text-muted">Acknowledged</span>
                )}
              </span>
            </li>
          ))}
        </ul>
      </div>
      <div className="card">
        <button className="btn" onClick={() => downloadCsv('sevenacademy-demo.csv', buildCsv())}>Export CSV</button>
      </div>
    </div>
  )
}


