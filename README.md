https://automation-dotlapse-artefact.lambdatest.com/org-486308/8d19cdd3-380b-4a0b-b952-d0dbb9ebf7d2/98686a44-f2e2-437b-bd6f-8e163f2c4f34/base-screenshots/328490c6-e247-454a-98ef-cd2f98ce8be7.png

https://automation-dotlapse-artefact.lambdatest.com/org-486308/8d19cdd3-380b-4a0b-b952-d0dbb9ebf7d2/f489538d-327d-4531-afe6-d185fa9feb77/base-screenshots/9ddc53ac-7a83-4098-9ebd-45665b4d5708.png

cpp binary build : g++ -std=c++17 -I/opt/homebrew/opt/opencv/include/opencv4 -L/opt/homebrew/opt/opencv/lib -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lcurl -O3 -march=native -o image_compare main.cpp 

go binary build : go build -ldflags="-s -w" -o image_compare main.go

binary command : ./image_compare 

