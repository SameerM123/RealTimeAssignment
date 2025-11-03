import { useEffect, useMemo, useState } from 'react'
import { getItemById, items } from '../lib/data'
import { grade, nextItem } from '../lib/rules'
import { useSessionStore, useStudentStore } from '../state/store'

type Phase = 'IDLE' | 'AWAIT_ANSWER' | 'EVALUATE' | 'FEEDBACK'

export function SessionPage() {
  const mode = useSessionStore((s) => s.mode)
  const currentItemId = useSessionStore((s) => s.currentItemId)
  const history = useSessionStore((s) => s.history)
  const setCurrentItemId = useSessionStore((s) => (id: string | null) => (s.currentItemId = id))
  const pushHistory = useSessionStore((s) => (entry: any) => (s.history = [...s.history, entry]))

  const mastery = useStudentStore((s) => s.masteryByConcept)
  const nudgeMastery = useStudentStore((s) => s.nudgeMastery)

  const [phase, setPhase] = useState<Phase>('IDLE')
  const [selected, setSelected] = useState<number | null>(null)
  const [result, setResult] = useState<{ correct: boolean; tag?: string } | null>(null)
  const [hintLevel, setHintLevel] = useState<'none' | 'basic' | 'targeted'>('none')

  useEffect(() => {
    // Initialize first item deterministically
    if (!currentItemId && items.length) {
      setCurrentItemId(items[0].id)
      setPhase('AWAIT_ANSWER')
    }
  }, [currentItemId, setCurrentItemId])

  const item = useMemo(() => (currentItemId ? getItemById(currentItemId) : null), [currentItemId])

  function onCheck() {
    if (!item) return
    const responseIndex = selected
    if (responseIndex == null) return
    setPhase('EVALUATE')
    const g = grade(item, responseIndex)
    setResult({ correct: g.correct, tag: g.misconceptionTag })
    const delta = g.correct ? 10 : -7
    nudgeMastery(item.concept_id, delta)
    pushHistory({ id: item.id, correct: g.correct, concept: item.concept_id, tag: g.misconceptionTag })
    // Hint unlocking logic
    if (!g.correct) {
      setHintLevel((prev) => (prev === 'none' ? 'basic' : 'targeted'))
    }
    setPhase('FEEDBACK')
  }

  function onNext() {
    if (!item) return
    const next = nextItem({ currentItemId, history, masteryByConcept: mastery })
    setCurrentItemId(next)
    setSelected(null)
    setResult(null)
    setPhase('AWAIT_ANSWER')
  }

  return (
    <div className="grid gap-4 md:grid-cols-[1fr_320px]">
      <section className="card" aria-labelledby="stem-label">
        <div id="stem-label" className="text-sm text-muted">Item</div>
        <div className="mt-2 text-lg font-medium">{item?.stem ?? '—'}</div>
        {item?.type === 'mcq' && (
          <div role="group" aria-label="Choices" className="mt-4 space-y-2">
            {item.choices.map((c: string, idx: number) => (
              <label key={c} className="flex cursor-pointer items-center gap-3 rounded-md border border-border bg-surface px-3 py-2 hover:bg-surface/70">
                <input
                  type="radio"
                  name="choice"
                  aria-label={`choice ${idx + 1}`}
                  checked={selected === idx}
                  onChange={() => setSelected(idx)}
                />
                <span>{c}</span>
              </label>
            ))}
          </div>
        )}
        {hintLevel !== 'none' && (
          <div className="mt-4 rounded-md border border-border bg-surface p-3 text-sm">
            <div className="text-muted">{hintLevel === 'basic' ? 'Hint (basic)' : 'Hint (targeted)'}</div>
            <div className="mt-1">{hintLevel === 'basic' ? item?.hint_basic : item?.hint_targeted}</div>
          </div>
        )}
        <div className="mt-4 flex gap-2">
          <button className="btn" onClick={onCheck} disabled={selected == null || phase !== 'AWAIT_ANSWER'}>
            Check Answer
          </button>
          <button className="btn" onClick={onNext} disabled={phase !== 'FEEDBACK' && mode !== 'quiz'}>
            Next
          </button>
        </div>
        {result && (
          <div role="status" className={`mt-3 inline-block rounded-md px-3 py-2 text-sm ${result.correct ? 'bg-success/20 text-success' : 'bg-error/20 text-error'}`}>
            {result.correct ? 'Correct!' : 'Incorrect'}
          </div>
        )}
      </section>
      <aside className="card" aria-labelledby="tutor-label">
        <div id="tutor-label" className="text-sm text-muted">Tutor Sidebar</div>
        <div className="mt-2 space-y-3">
          <div>
            <div className="text-sm text-muted">Mastery</div>
            <div className="mt-2 space-y-2">
              {Object.entries(mastery).map(([concept, value]) => (
                <div key={concept} className="flex items-center gap-2">
                  <div className="w-40 truncate text-sm text-muted">{concept}</div>
                  <div className="h-2 w-full rounded bg-border">
                    <div className="h-2 rounded bg-brand" style={{ width: `${value}%` }} />
                  </div>
                  <div className="w-10 text-right text-sm">{Math.round(value)}%</div>
                </div>
              ))}
            </div>
          </div>
          <div>
            <div className="text-sm text-muted">Next Up</div>
            <div className="mt-1 text-sm">Based on rules and recent outcome</div>
          </div>
        </div>
      </aside>
    </div>
  )
}


