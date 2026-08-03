# release build: -DNDEBUG ตัด assert ออกทุกไฟล์พร้อมกัน (lsb() เป็น inline ใน header
# ถ้าใส่ไม่ครบทุก TU จะผิด ODR) ถ้าจะดีบักให้ใช้ build_tests.ps1 ที่เปิด assert ไว้
g++ -O2 -std=c++17 -Wall -Wextra -flto -DNDEBUG -o engine.exe main.cpp position.cpp attacks.cpp movegen.cpp
if ($?) { .\engine.exe }
