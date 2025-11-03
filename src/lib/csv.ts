import { roster } from './data'

export function buildCsv(): string {
  const header = 'student_id,name,concept,mastery\n'
  const lines = roster.flatMap((r) =>
    Object.entries(r.mastery).map(([concept, value]) => `${r.student_id},${r.name},${concept},${value}`)
  )
  return header + lines.join('\n') + '\n'
}

export function downloadCsv(filename: string, content: string) {
  const blob = new Blob([content], { type: 'text/csv;charset=utf-8;' })
  const url = URL.createObjectURL(blob)
  const a = document.createElement('a')
  a.href = url
  a.download = filename
  a.click()
  URL.revokeObjectURL(url)
}


