# История разработки
Тут описаны даты и зафиксированные важные нововведения в движок.

Меню всех дат:
- [21 ноября 2025](#211125)
- [13 декабря 2025](#131225)
- [14 декабря 2025](#141225)
- [17 декабря 2025](#171225)
- [18 декабря 2025](#181225)
- [27 декабря 2025](#271225)

#

### 21.11.25
Просто зафиксировал дату создания проекта:</br>

<img width="367" height="504" alt="1" src="https://github.com/user-attachments/assets/f7c84da6-c0b4-4ab1-a7c6-b96cdf75cb30" />

#

### 13.12.25
Пре-база движка. Были добавлены контроллеры 2D и 3D камеры.</br>

CameraController2D</br>
![2](https://github.com/user-attachments/assets/603ef5f6-ab98-4930-aea7-45e4c4613448)</br>

CameraController3D</br>
_Не показано но он есть_

CameraOrbitController3D</br>
![3](https://github.com/user-attachments/assets/f05cadb6-ff9b-454e-aa62-2cdf74c6c533)</br>

#

### 14.12.25
Просто разные эксперименты. Ничего важного.

![1](https://github.com/user-attachments/assets/eb819faa-dfe7-47ea-bc72-8903988dea61)
![2](https://github.com/user-attachments/assets/7fd9db85-cd86-4a49-a8c1-a747615012e9)
![3](https://github.com/user-attachments/assets/51073b54-ab13-4dec-8628-15ba220a158b)

#

### 17.12.25
Добавление поддержки кадрового буфера. На видео черная область - фреймбуфер который не меняет свой размер.</br>
Орбитальный шарик рисуется только во фреймбуфере и виден только из черной области окна.</br>

![1](https://github.com/user-attachments/assets/43510f5e-738b-4490-a532-11ef2411fb17)
 
#

### 18.12.25
Наконец! Спустя столько времени я смог получить карту глубины. Я очень рад. Вот кучу демонстративного контента:</br>

![1](https://github.com/user-attachments/assets/5d69d4ef-1f16-40f8-8509-a101a1651011)
![2](https://github.com/user-attachments/assets/1b415888-ed4e-4f91-a4ae-69cbdff05acc)
![3](https://github.com/user-attachments/assets/96608984-ea79-4efc-8486-3dbc72074e87)
![4](https://github.com/user-attachments/assets/84ff385b-8dd7-46e3-bcd1-0a5a36a8fac6)

Также я экспериментировал с новой функцией в 3д камере: `look_at()`:</br>

![5](https://github.com/user-attachments/assets/6ed3b2a3-4b41-475f-b153-84427e313cfd)
![6](https://github.com/user-attachments/assets/23a4ca59-d7d0-489d-9680-24cd2fcb7a45)
![7](https://github.com/user-attachments/assets/963c22eb-a508-4071-a2de-b7d15f6abf54)

#

### 27.12.25
Сделал простой парсер OBJ файлов. Надо его доделать и сделать поддержку загрузки всех отдельных моделей в файле, и поддержку загрузки материалов и их привязку к необходимым сеткам. В общем ещё много работы:</br>

![1](https://github.com/user-attachments/assets/d19eeab8-51b8-4e7a-ab51-f9cd8f99276e)
![2](https://github.com/user-attachments/assets/34343ab3-941a-4468-b9db-44b691e57f25)
![3](https://github.com/user-attachments/assets/dd1b58e1-ac30-43fc-999d-c79b5094d29e)
![7](https://github.com/user-attachments/assets/f9f12264-f4a6-40fc-9f0b-ba5ff1485286)

Также получилось наглядно увидеть работу MipMap текстуры:</br>

<img width="1914" height="1005" alt="4" src="https://github.com/user-attachments/assets/000413c6-2111-4f2f-a65c-89fec698b504" />
<img width="1916" height="1005" alt="5" src="https://github.com/user-attachments/assets/8c638b56-0999-41c9-9e6f-d0359b18091b" />
<img width="1916" height="997" alt="6" src="https://github.com/user-attachments/assets/e3d69677-d4f9-4cd6-bf1e-1a9acbf28258" />

#

На этот 2025 год это всё.</br>
Следующие изменения будут в другом файле.</br>
