import { useEffect } from 'react'
import { useLocation } from 'react-router-dom'
import { findSeed } from '../lib/data'
import { usePlannerStore, useSessionStore, useStudentStore } from '../state/store'

export function SeedInitializer({ children }: { children: React.ReactNode }) {
  const loc = useLocation()
  const setMode = useSessionStore((s) => s.setMode)
  const setIntensity = usePlannerStore((s) => s.setIntensity)
  const setMastery = useStudentStore((s) => s.nudgeMastery)

  useEffect(() => {
    const p = new URLSearchParams(loc.search)
    const seedId = p.get('seed') ?? undefined
    const mode = (p.get('mode') as 'guided' | 'quiz' | null) ?? null
    const intensity = (p.get('intensity') as 'light' | 'standard' | 'intense' | null) ?? null

    if (mode) setMode(mode)
    if (intensity) setIntensity(intensity)

    const seed = findSeed(seedId)
    if (seed?.override_mastery) {
      Object.entries(seed.override_mastery).forEach(([concept, value]) => {
        // nudgeMastery is delta-based; set absolute by diffing from 0 baseline
        setMastery(concept, Number(value))
      })
    }
  }, [loc.search, setIntensity, setMastery, setMode])

  return <>{children}</>
}


