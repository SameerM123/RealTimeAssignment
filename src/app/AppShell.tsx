import { AppRouter } from './router'

export function AppShell() {
  return (
    <div className="min-h-screen bg-bg text-text">
      <header className="sticky top-0 z-10 border-b border-border bg-surface/95 backdrop-blur">
        <div className="mx-auto flex max-w-6xl items-center justify-between px-4 py-3">
          <a href="/" className="font-semibold text-white">Se7enAcademy</a>
          <nav className="text-sm text-muted">
            <a className="hover:text-white" href="/student">Student</a>
            <span className="mx-3">•</span>
            <a className="hover:text-white" href="/teacher">Teacher</a>
            <span className="mx-3">•</span>
            <a className="hover:text-white" href="/concepts">Concepts</a>
            <span className="mx-3">•</span>
            <a className="hover:text-white" href="/about">About</a>
          </nav>
        </div>
      </header>
      <main className="mx-auto max-w-6xl px-4 py-6">
        <AppRouter />
      </main>
    </div>
  )
}


