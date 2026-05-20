#!/bin/bash

# Предполагается, что ваш сервер запущен на localhost:8080

# --- Тест 1: 404 Not Found ---
echo "--- 1. Тестируем 404 Not Found (файл не существует) ---"
curl -i http://localhost:8080/this_file_does_not_exist.html
echo -e "\n"

# --- Тест 2: 404 Not Found (в директории нет index) ---
echo "--- 2. Тестируем 404 Not Found (в директории нет index-файла) ---"
mkdir -p www/empty_dir
curl -i http://localhost:8080/empty_dir/
rm -rf www/empty_dir
echo -e "\n"

# --- Тест 3: 403 Forbidden ---
echo "--- 3. Тестируем 403 Forbidden (нет прав на чтение) ---"
touch www/secret.html
echo "you should not see this" > www/secret.html
chmod 200 www/secret.html # Устанавливаем права "только запись" для владельца
curl -i http://localhost:8080/secret.html
rm www/secret.html # Очистка
echo -e "\n"

# --- Тест 4: 405 Method Not Allowed ---
echo "--- 4. Тестируем 405 Method Not Allowed (недопустимый метод) ---"
# В вашем конфиге для location / разрешены GET, POST, DELETE.
# Отправляем PUT, который должен быть отклонен.
curl -i -X PUT http://localhost:8080/index.html
echo -e "\n"

# --- Тест 5: 200 OK (для сравнения) ---
echo "--- 5. Тестируем 200 OK (успешный запрос) ---"
curl -i http://localhost:8080/index.html
echo -e "\n"
