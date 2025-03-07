default:
    just backend & just frontend & wait

backend: compile
    cd backend && ./build/Main

frontend:
    cd frontend && bun run dev

compile:
    cd backend && cmake -B build && cmake --build build

