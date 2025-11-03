import { create } from 'zustand'

export type SessionMode = 'guided' | 'quiz'

type HistoryEntry = { id: string; correct: boolean; concept: string; tag?: string }

type SessionState = {
  currentItemId: string | null
  mode: SessionMode
  history: HistoryEntry[]
  setMode: (mode: SessionMode) => void
}

export const useSessionStore = create<SessionState>((set) => ({
  currentItemId: null,
  mode: 'guided',
  history: [],
  setMode: (mode) => set({ mode }),
}))

type StudentState = {
  masteryByConcept: Record<string, number>
  nudgeMastery: (concept: string, delta: number) => void
}

export const useStudentStore = create<StudentState>((set) => ({
  masteryByConcept: {},
  nudgeMastery: (concept, delta) =>
    set((s) => {
      const current = s.masteryByConcept[concept] ?? 0
      const next = Math.max(0, Math.min(100, current + delta))
      return { masteryByConcept: { ...s.masteryByConcept, [concept]: next } }
    }),
}))

type PlannerState = {
  intensity: 'light' | 'standard' | 'intense'
  plan: Array<{ concept: string; date: string; reason: string }>
  setIntensity: (v: 'light' | 'standard' | 'intense') => void
}

export const usePlannerStore = create<PlannerState>((set) => ({
  intensity: 'standard',
  plan: [],
  setIntensity: (v) => set({ intensity: v }),
}))

type TeacherState = {
  roster: Array<{ student_id: string; name: string; mastery: Record<string, number>; flags?: string[] }>
}

export const useTeacherStore = create<TeacherState>(() => ({
  roster: [],
}))


