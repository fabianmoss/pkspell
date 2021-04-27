import pickle
import random
from collections import Counter
from pathlib import Path

import click
import numpy as np
import sklearn
import sklearn.model_selection
import torch
from torch.utils.data import DataLoader

from pitches import keep_best_transpositions
from utils import root_folder
from datasets import (
    N_DURATION_CLASSES,
    PSDataset,
    pad_collate,
    ks_to_ix,
    midi_to_ix,
    pitch_to_ix,
    transform_chrom,
    transform_diat,
    transform_key,
)

# Reproducibility
# See https://pytorch.org/docs/stable/notes/randomness.html
seed = 42
random.seed(seed)
torch.manual_seed(seed)
np.random.seed(seed)
# torch.use_deterministic_algorithms(True)
torch.backends.cudnn.benchmark = False

basepath = "./"  # to change if running locally or on colab
# load the asap datasets with ks
with open(Path("./asapks.pkl"), "rb") as fid:
    full_dict_dataset = pickle.load(fid)

# ####### Note for Nicolas: I called it "dict_dataset", but it is a list of dictionaries
paths = list(set([e["original_path"] for e in full_dict_dataset]))

dict_dataset = keep_best_transpositions(full_dict_dataset)
composer_per_piece = [root_folder(p) for p in paths]
c = Counter(composer_per_piece)

# remove pieces from asap that are in Musedata
print(len(paths), "initial pieces")
paths = [p for p in paths if p != "Bach/Prelude/bwv_865/xml_score.musicxml"]
# remove mozart Fantasie because of incoherent key signature
paths = [p for p in paths if p != "Mozart/Fantasie_475/xml_score.musicxml"]

print(len(paths), "pieces after removing overlapping with musedata and Mozart Fantasie")

paths = sorted(paths)

# Divide train and validation set
composer_per_piece = [root_folder(p) for p in paths]

path_train, path_validation = sklearn.model_selection.train_test_split(
    # paths, test_size=0.15, stratify=composer_per_piece, random_state=42
    paths,
    test_size=0.15,
    random_state=42,
)
print(path_validation)
print("Train and validation lenghts: ", len(path_train), len(path_validation))

# need to find a better way to visualize this
composers = list(set([root_folder(p) for p in paths]))
print(f"Remaining composers: {composers}")

train_composer = [composers.index(root_folder(p)) for p in path_train]
val_composer = [composers.index(root_folder(p)) for p in path_validation]


# %%
@click.command()
@click.option("--model", default="RNNMulti", type=str)
@click.option("--epochs", default=30, type=int)
@click.option("--lr", default=0.05, type=float)
@click.option("--momentum", default=0.9, type=float)
@click.option("--decay", default=1e-4, type=float)
@click.option("--hidden_dim", default=100, type=int)
@click.option("--hidden_dim2", default=50, type=int)
@click.option("--bs", default=8, type=int)
@click.option("--layers", default=1, type=int)
@click.option("--device", type=str)
@click.option("--dropout", default=0, type=float)
@click.option("--cell", default="GRU", type=str)
@click.option("--optimizer", default="SGD", type=str)
@click.option("--learn_all", is_flag=True, default=False)
@click.option("--bidirectional", default=True, type=bool)
@click.option("--mode", default="both", type=str)
def train_pitch_speller(
    model,
    epochs,
    lr,
    hidden_dim,
    bs,
    momentum,
    hidden_dim2,
    layers,
    device,
    dropout,
    cell,
    decay,
    optimizer,
    learn_all,
    bidirectional,
    mode,
):
    MODEL = model
    N_EPOCHS = epochs
    HIDDEN_DIM = hidden_dim
    LEARNING_RATE = lr
    WEIGHT_DECAY = decay
    BATCH_SIZE = bs
    MOMENTUM = momentum
    RNN_LAYERS = layers
    DEVICE = device
    DROPOUT = dropout
    RNN_CELL = cell
    OPTIMIZER = optimizer
    BIDIRECTIONAL = bidirectional
    MODE = mode

    # ks rnn hyperparameter
    HIDDEN_DIM2 = hidden_dim2

    # attention hyperparameter
    NUM_HEAD = 2
    NUM_LANDMARKS = 64

    if learn_all:
        print("Learning from full dataset")
        train_dataset = PSDataset(
            dict_dataset,
            path_train + path_validation,
            transform_chrom,
            transform_diat,
            transform_key,
            True,
            sort=True,
            truncate=None,
        )
        validation_dataset = None
        val_dataloader = None
    else:
        train_dataset = PSDataset(
            dict_dataset,
            path_train,
            transform_chrom,
            transform_diat,
            transform_key,
            True,
            sort=True,
            truncate=None,
        )
        validation_dataset = PSDataset(
            dict_dataset,
            path_validation,
            transform_chrom,
            transform_diat,
            transform_key,
            False,
        )
        val_dataloader = DataLoader(
            validation_dataset,
            batch_size=1,
            shuffle=False,
            collate_fn=pad_collate,
            num_workers=1,
        )

    train_dataloader = DataLoader(
        train_dataset,
        batch_size=BATCH_SIZE,
        shuffle=True,
        collate_fn=pad_collate,
        num_workers=2,
    )

    from models import RNNMultiTagger  # RNNMultNystromAttentionTagger,
    from models import RNNNystromAttentionTagger, RNNTagger

    if model == "RNN":
        model = RNNTagger(
            len(midi_to_ix) + N_DURATION_CLASSES,
            HIDDEN_DIM,
            pitch_to_ix,
            ks_to_ix,
            n_layers=RNN_LAYERS,
            dropout=DROPOUT,
            cell_type=RNN_CELL,
            bidirectional=BIDIRECTIONAL,
            mode=MODE,
        )
    elif model == "RNNMulti":
        model = RNNMultiTagger(
            len(midi_to_ix) + N_DURATION_CLASSES,
            HIDDEN_DIM,
            pitch_to_ix,
            ks_to_ix,
            n_layers=RNN_LAYERS,
            dropout=DROPOUT,
            hidden_dim2=HIDDEN_DIM2,
            cell_type=RNN_CELL,
            bidirectional=BIDIRECTIONAL,
            mode=MODE,
        )
    elif model == "Nystrom":
        model = RNNNystromAttentionTagger(
            len(midi_to_ix) + N_DURATION_CLASSES,
            HIDDEN_DIM,
            pitch_to_ix,
            ks_to_ix,
            n_layers=RNN_LAYERS,
            num_head=NUM_HEAD,
            num_landmarks=NUM_LANDMARKS,
            bidirectional=BIDIRECTIONAL,
            mode=MODE,
        )
        # model = RNNMultNystromAttentionTagger(
        #     len(midi_to_ix) + N_DURATION_CLASSES,
        #     HIDDEN_DIM,
        #     pitch_to_ix,
        #     ks_to_ix,
        #     n_layers=RNN_LAYERS,
        #     hidden_dim2=HIDDEN_DIM2,
        #     num_head=NUM_HEAD,
        #     num_landmarks=NUM_LANDMARKS,
        # )

    from torch import optim
    from torch.optim import lr_scheduler

    if OPTIMIZER == "SGD":
        optimizer = optim.SGD(
            model.parameters(),
            lr=LEARNING_RATE,
            momentum=MOMENTUM,
            weight_decay=WEIGHT_DECAY,
        )
        scheduler = lr_scheduler.MultiStepLR(
            optimizer, [N_EPOCHS // 2], gamma=0.1, verbose=True
        )
        # scheduler = lr_scheduler.ReduceLROnPlateau(optimizer, mode='max', factor=0.1, patience=N_EPOCHS/10, verbose=True)
    elif OPTIMIZER == "Adam":
        optimizer = optim.Adam(
            model.parameters(), lr=LEARNING_RATE, weight_decay=WEIGHT_DECAY
        )
        scheduler = lr_scheduler.MultiStepLR(
            optimizer, [N_EPOCHS // 2], gamma=0.1, verbose=True
        )

    from torch.utils.tensorboard import SummaryWriter

    from train import training_loop

    hyperparams_str = f"{RNN_CELL}{MODEL}_{OPTIMIZER}_lr-{LEARNING_RATE}_nlayers-{RNN_LAYERS}_bs-{BATCH_SIZE}_dim-{HIDDEN_DIM}_dropout-{DROPOUT}_bidirectional-{BIDIRECTIONAL}_mode_{MODE}"
    print(hyperparams_str)
    writer = SummaryWriter(comment="_" + hyperparams_str, flush_secs=20)

    history = training_loop(
        model,
        optimizer,
        train_dataloader,
        epochs=N_EPOCHS,
        val_dataloader=val_dataloader,
        writer=writer,
        device=device,
        scheduler=scheduler,
    )


if __name__ == "__main__":
    train_pitch_speller()
