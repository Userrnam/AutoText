# A simple UI for rwkv LLM.

## Building:

git clone --recursive git@github.com:Userrnam/AutoText.git

cd AutoText/tokenizer

cargo build

mkdir ../.build

cd ../.build

cmake ..

cmake --build .

## Downloading an RWKV model

#### Option 1. Download pre-quantized Raven model

There are pre-quantized Raven models available on [Hugging Face](https://huggingface.co/BlinkDL/rwkv-4-raven/tree/main). Check that you are downloading `.bin` file, **not** `.pth`.

#### Option 2. Convert and quantize PyTorch model

**Requirements**: Python 3.x with [PyTorch](https://pytorch.org/get-started/locally/).

This option would require a little more manual work, but you can use it with any RWKV model and any target format.

**First**, download a model from [Hugging Face](https://huggingface.co/BlinkDL) like [this one](https://huggingface.co/BlinkDL/rwkv-4-pile-169m/blob/main/RWKV-4-Pile-169M-20220807-8023.pth).

**Second**, convert it into `rwkv.cpp` format using following commands:

```commandline
# Windows
python python\convert_pytorch_to_ggml.py C:\RWKV-4-Pile-169M-20220807-8023.pth C:\rwkv.cpp-169M.bin FP16

# Linux / MacOS
python python/convert_pytorch_to_ggml.py ~/Downloads/RWKV-4-Pile-169M-20220807-8023.pth ~/Downloads/rwkv.cpp-169M.bin FP16
```

**Optionally**, quantize the model into one of quantized formats from the table above:

```commandline
# Windows
python python\quantize.py C:\rwkv.cpp-169M.bin C:\rwkv.cpp-169M-Q5_1.bin Q5_1

# Linux / MacOS
python python/quantize.py ~/Downloads/rwkv.cpp-169M.bin ~/Downloads/rwkv.cpp-169M-Q5_1.bin Q5_1
```

Put the downloaded model into the models folder.
