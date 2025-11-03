import itemsJson from '../data/items.json'
import conceptsJson from '../data/concepts.json'
import rulesJson from '../data/rules.json'
import rosterJson from '../data/class_roster.json'
import seedsJson from '../data/seeds.json'

export type Item = typeof itemsJson[number]
export type Concept = typeof conceptsJson[number]
export type Rules = typeof rulesJson
export type RosterEntry = typeof rosterJson[number]
export type Seed = typeof seedsJson[number]

export const items: Item[] = itemsJson
export const concepts: Concept[] = conceptsJson
export const rules: Rules = rulesJson
export const roster: RosterEntry[] = rosterJson
export const seeds: Seed[] = seedsJson

export function getConceptById(id: string) {
  return concepts.find((c) => c.id === id)
}

export function getItemById(id: string) {
  return items.find((i) => i.id === id)
}

export function getItemsByConcept(conceptId: string) {
  return items.filter((i) => i.concept_id === conceptId)
}

export function colorForMastery(value: number) {
  const w = rules.thresholds.weak
  const a = rules.thresholds.advance
  if (value < w) return 'bg-error/20 text-error'
  if (value < a) return 'bg-warn/20 text-warn'
  return 'bg-success/20 text-success'
}

export function findSeed(id?: string) {
  if (!id) return undefined
  return seeds.find((s) => s.id === id)
}


