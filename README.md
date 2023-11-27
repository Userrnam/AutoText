# A simple UI for rwkv LLM.

## Building:

git clone --recursive git@github.com:Userrnam/AutoText.git

cd AutoText/tokenizer

cargo build

mkdir ../.build

cd ../.build

cmake ..

cmake --build .
