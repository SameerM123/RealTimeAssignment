import type { Config } from 'tailwindcss'

export default {
  content: [
    './index.html',
    './src/**/*.{ts,tsx}',
  ],
  theme: {
    extend: {
      colors: {
        brand: {
          DEFAULT: '#1d4ed8',
          ink: '#0b2a6b',
        },
        bg: '#0b0e14',
        surface: '#0f1420',
        border: '#2a3345',
        text: '#e6edf3',
        muted: '#9fb1c7',
        success: '#16a34a',
        warn: '#d97706',
        error: '#dc2626',
      },
    },
  },
  plugins: [],
} satisfies Config


