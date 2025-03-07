default:
    just backend & just frontend & wait

backend: compile-backend
    cd backend && ./build/Main

frontend:
    cd frontend && bun run dev

test: compile-tests
    cd backend && ./build/Tests

compile-tests:
    cd backend && cmake --build build --target Tests

compile-backend:
    cd backend && cmake --build build --target Main

setup-frontend:
    cd frontend && bun install

setup-backend:
    cd backend && cmake -B build