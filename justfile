default:
    just backend & just frontend & wait

backend: compile
    cd backend && ./build/Main

frontend:
    cd frontend && bun run dev

test: compile
    cd backend && ./build/Tests

compile:
    cd backend && cmake --build build 

setup-frontend:
    cd frontend && bun install

setup-backend:
    cd backend && cmake -B build
