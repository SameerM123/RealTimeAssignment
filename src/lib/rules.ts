import { concepts, getItemsByConcept, items, rules } from './data'

export type GradeResult = { correct: boolean; reason?: string; misconceptionTag?: string }

export function grade(item: any, response: any): GradeResult {
  if (item?.type === 'mcq') {
    const correct = response === item?.answer_index
    const misconceptionTag = !correct && item?.misconception_key_indexes?.includes(response)
      ? 'misconception:stroma'
      : undefined
    return { correct, misconceptionTag, reason: correct ? 'exact_match' : 'mismatch' }
  }
  if (item?.type === 'short') {
    const norm = String(response ?? '').trim().toLowerCase()
    const correct = norm === String(item?.answer ?? '').trim().toLowerCase()
    return { correct, reason: correct ? 'exact_match' : 'mismatch' }
  }
  return { correct: false, reason: 'unsupported_item_type' }
}

export function nextItem(state: {
  currentItemId: string | null
  history: Array<{ id: string; correct: boolean; concept: string; tag?: string }>
  masteryByConcept: Record<string, number>
}): string | null {
  const thresholds = rules.thresholds
  const last = state.history[state.history.length - 1]
  const lastConcept = last?.concept ?? items[0]?.concept_id
  if (!lastConcept) return items[0]?.id ?? null

  const mastery = state.masteryByConcept[lastConcept] ?? 0
  const conceptItems = getItemsByConcept(lastConcept)

  // If misconception tag fired or mastery below weak, keep in same concept, pick easiest available
  const hadMisconception = last?.tag?.startsWith('misconception:')
  if (hadMisconception || mastery < thresholds.weak) {
    const sorted = [...conceptItems].sort((a, b) => (a.difficulty ?? 1) - (b.difficulty ?? 1))
    return sorted[0]?.id ?? conceptItems[0]?.id ?? items[0]?.id ?? null
  }

  // If good mastery and two correct in a row, advance to next concept with prerequisites met
  const lastTwo = state.history.slice(-2)
  const twoCorrect = lastTwo.length === 2 && lastTwo.every((h) => h.correct)
  if (mastery >= thresholds.advance && twoCorrect) {
    const idx = concepts.findIndex((c) => c.id === lastConcept)
    const nextConcept = concepts[idx + 1]
    if (nextConcept) {
      const nextItems = getItemsByConcept(nextConcept.id)
      if (nextItems.length) return nextItems[0].id
    }
  }

  // Otherwise stay in concept, try higher difficulty if exists
  const sortedDesc = [...conceptItems].sort((a, b) => (b.difficulty ?? 1) - (a.difficulty ?? 1))
  return sortedDesc[0]?.id ?? conceptItems[0]?.id ?? items[0]?.id ?? null
}

export function updateMastery(masteryByConcept: Record<string, number>, concept: string, delta: number) {
  const current = masteryByConcept[concept] ?? 0
  const next = Math.max(0, Math.min(100, current + delta))
  return { ...masteryByConcept, [concept]: next }
}

export type PlanEntry = { concept: string; date: string; reason: string }

export function rebuildSpacedPlan(masteryByConcept: Record<string, number>, intensity: 'light' | 'standard' | 'intense'): PlanEntry[] {
  const intervals = rules.spaced_intervals_days[intensity]
  const today = new Date()
  const entries: PlanEntry[] = []
  Object.entries(masteryByConcept).forEach(([concept, mastery]) => {
    const reason = mastery < rules.thresholds.weak ? 'weak' : 'verify'
    intervals.forEach((d) => {
      const dt = new Date(today)
      dt.setDate(today.getDate() + d)
      const iso = dt.toISOString().slice(0, 10)
      entries.push({ concept, date: iso, reason })
    })
  })
  // Keep within next 14 days and deterministic order
  return entries
    .filter((e) => {
      const target = new Date(e.date)
      const diffDays = (target.getTime() - today.getTime()) / (1000 * 60 * 60 * 24)
      return diffDays >= 0 && diffDays <= 14
    })
    .sort((a, b) => (a.date < b.date ? -1 : a.date > b.date ? 1 : a.concept.localeCompare(b.concept)))
}

export function explainability(reasonCodes: string[]) {
  return reasonCodes.join(' · ')
}


