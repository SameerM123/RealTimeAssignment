import { BrowserRouter, Routes, Route, Navigate } from 'react-router-dom'
import { LandingPage } from '../pages/Landing'
import { StudentHomePage } from '../pages/StudentHome'
import { SessionPage } from '../pages/Session'
import { PlannerPage } from '../pages/Planner'
import { TeacherPage } from '../pages/Teacher'
import { ConceptsPage } from '../pages/Concepts'
import { AboutPage } from '../pages/About'
import { SeedInitializer } from './SeedInitializer'

export function AppRouter() {
  return (
    <BrowserRouter>
      <SeedInitializer>
        <Routes>
          <Route path="/" element={<LandingPage />} />
          <Route path="/student" element={<StudentHomePage />} />
          <Route path="/student/session" element={<SessionPage />} />
          <Route path="/student/plan" element={<PlannerPage />} />
          <Route path="/teacher" element={<TeacherPage />} />
          <Route path="/concepts" element={<ConceptsPage />} />
          <Route path="/about" element={<AboutPage />} />
          <Route path="*" element={<Navigate to="/" replace />} />
        </Routes>
      </SeedInitializer>
    </BrowserRouter>
  )
}


