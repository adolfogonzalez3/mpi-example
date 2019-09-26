#include <Eigen/Dense>
#include <iostream>
#include <time.h>
#include <mpi.h>
#include "mnist/mnist_reader.hpp"
#include "mnist/mnist_utils.hpp"

using Eigen::MatrixXd;
using Eigen::Map;
using namespace std;

using Dataset =
    mnist::MNIST_dataset<std::vector, std::vector<uint8_t>, uint8_t>;

// Perform argmax on a Matrix
// Retrieves the index of the maximum argument for each row
MatrixXd argmax(const MatrixXd &matrix) {
  MatrixXd returned_value(matrix.rows(), 1);
  int index = 0;
  for (int i = 0; i < matrix.rows(); i++) {
    matrix.row(i).maxCoeff(&index);
    returned_value(i, 0) = index;
  }
  return std::move(returned_value);
}

// Load a batch from a 2d vector
template <class T>
MatrixXd load_batch2D(const vector<vector<T>> &data, unsigned int from,
                      unsigned int to) {
  const unsigned int BATCH_SIZE = to - from;
  const unsigned int WIDTH = data[0].size();
  MatrixXd batch(BATCH_SIZE, WIDTH);
  for (int i = 0; i < BATCH_SIZE; i++) {
    for (int j = 0; j < WIDTH; j++) {
      batch(i, j) = data[from + i][j];
    }
  }
  return std::move(batch / 256);
}

template <class T>
MatrixXd load_batch1D(const vector<T> &data, unsigned int from,
                      unsigned int to) {
  const unsigned int BATCH_SIZE = to - from;
  MatrixXd batch(BATCH_SIZE, 1);
  for (int i = 0; i < BATCH_SIZE; i++) {
    batch(i, 0) = data[from + i];
  }
  return std::move(batch);
}

MatrixXd to_onehot(const MatrixXd &matrix, const unsigned labels) {
  MatrixXd batch(matrix.rows(), labels);
  for (int i = 0; i < matrix.rows(); i++) {
    for (int j = 0; j < labels; j++)
      batch(i, j) = 0;
    batch(i, matrix(i, 0)) = 1;
  }
  return std::move(batch);
}

class Tensor {
public:
  /*
  MatrixXd operator()(const MatrixXd &input) const {
    return std::move(forward(input));
  }
  */
  virtual MatrixXd forward(const MatrixXd &input) const {
    cout << "WHY?" << endl;
  }

  MatrixXd backward(const MatrixXd &input) const {
    MatrixXd tmp = forward(input);
    return std::move(-tmp.array() * (1 - tmp.array()));
  }

  virtual MatrixXd backward_with(const MatrixXd &input) const {};

  virtual MatrixXd backward_with(const MatrixXd &input,
                                 const MatrixXd &grad) const {};

  virtual void update_add(const MatrixXd &update){};
};

class Sigmoid : public Tensor {
public:
  MatrixXd forward(const MatrixXd &input) const {
    return 1 / (1 + (-input.array()).exp());
  }

  MatrixXd backward_with(const MatrixXd &input) const {
    MatrixXd output = forward(input);
    return -output.array() * (1 - output.array());
  }

  MatrixXd backward_with(const MatrixXd &input, const MatrixXd &grad) const {
    MatrixXd output = forward(input);
    return output.array() * (1 - output.array()) * grad.array();
  }
};

class Dense : public Tensor {
private:
  MatrixXd parameters;

public:
  Dense(unsigned int input, unsigned output) {
    parameters = MatrixXd(input, output);
    parameters.setRandom();
    parameters = (parameters * (6 / (input + output))).array().sqrt();
  }

  MatrixXd forward(const MatrixXd &input) const { return input * parameters; }

  MatrixXd backward_with(const MatrixXd &input, const MatrixXd &grad) const {
    return -input.transpose() * grad;
  }

  void update_add(const MatrixXd &update) { parameters = parameters + update; }
};

class Network {
private:
  vector<MatrixXd> forwards;
  vector<unique_ptr<Tensor>> tensors;

public:
  MatrixXd operator()(const MatrixXd &input) { return forward(input); }

  void push(unique_ptr<Tensor> tensor) { tensors.push_back(std::move(tensor)); }

  MatrixXd forward(const MatrixXd &matrix) {
    forwards.clear();
    forwards.push_back(matrix);
    for (int i = 0; i < tensors.size(); i++) {
      forwards.push_back(std::move(tensors[i]->forward(forwards.back())));
    }
    MatrixXd ret = std::move(forwards.back());
    forwards.pop_back();
    return std::move(ret);
  }

  vector<MatrixXd> gradients(const MatrixXd &grad) const {
    vector<MatrixXd> grads;
    grads.push_back(grad);
    // cout << "HELLO " << i << " " << forwards.size() << endl;
    for (int i = tensors.size() - 1; i >= 0; i--) {
      grads.push_back(tensors[i]->backward_with(forwards[i], grads.back()));
    }
    reverse(grads.begin(), grads.end());
    grads.pop_back();
    return std::move(grads);
  }

  void update_add(const vector<MatrixXd> &updates) {
    for (int i = 0; i < tensors.size(); i++) {
      // cout << "updates " << i << " " << updates[i].rows() << " " <<
      // updates[i].cols() << endl;
      tensors[i]->update_add(-0.001 * updates[i]);
    }
  }
};

int main(int argc, char *argv[]) {
  const unsigned int BATCH_SIZE = 32;
  const unsigned int FEATURES = 784;
  const unsigned int LABELS = 10;
  const unsigned int EPOCHS = 5;

  const int root = 0;
  int myrank;
  int size;

  time_t begin = time(NULL);

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  Dataset dataset =
      mnist::read_dataset<std::vector, std::vector, uint8_t, uint8_t>(".");
  // mnist::normalize_dataset(dataset);

  const unsigned int PART_SIZE = dataset.training_images.size() / size;
  const unsigned int LOW = PART_SIZE * myrank;
  const unsigned int HIGH = LOW + PART_SIZE;
  const unsigned int MOD = PART_SIZE / BATCH_SIZE;
  const unsigned int ITERATIONS = EPOCHS * MOD;
  const unsigned int LOG = ITERATIONS / 10;

  if (myrank == 0) {
    std::cout << "Nbr of training images = " << dataset.training_images.size()
              << std::endl;
    std::cout << "Nbr of training labels = " << dataset.training_labels.size()
              << std::endl;
    std::cout << "Nbr of test images = " << dataset.test_images.size()
              << std::endl;
    std::cout << "Nbr of test labels = " << dataset.test_labels.size()
              << std::endl;
    cout << dataset.training_images[0].size() << endl;
  }

  MatrixXd batch_x = load_batch2D(dataset.training_images, 0, BATCH_SIZE);
  MatrixXd batch_y_ = load_batch1D(dataset.training_labels, 0, BATCH_SIZE);
  MatrixXd batch_y = to_onehot(batch_y_, 10);

  MatrixXd result(BATCH_SIZE, LABELS);
  MatrixXd cost(BATCH_SIZE, LABELS);
  MatrixXd gradient(FEATURES, LABELS);

  Network network;
  network.push(unique_ptr<Dense>(new Dense(784, 10)));
  network.push(unique_ptr<Sigmoid>(new Sigmoid()));
  int index = 0;
  float before_cost, after_cost;
  result = network(batch_x);
  for (int i = 0; i < ITERATIONS; i++) {
    index = i % MOD;
    batch_x = load_batch2D(dataset.training_images, BATCH_SIZE * index,
                           BATCH_SIZE * (index + 1));
    batch_y_ = load_batch1D(dataset.training_labels, BATCH_SIZE * index,
                            BATCH_SIZE * (index + 1));
    batch_y = to_onehot(batch_y_, 10);
    result = network(batch_x);
    cost = (batch_y.array() - result.array());
    before_cost = (cost.array().pow(2) / 2).mean();
    auto gradients = network.gradients(cost);
    for (MatrixXd &grad : gradients) {
      double* what = new double[grad.size()];
      double* who = new double[grad.size()];
      Map<MatrixXd>( what, grad.rows(), grad.cols() ) =   grad;

      MPI_Allreduce(what, who, grad.size(), MPI_DOUBLE, MPI_SUM,
                    MPI_COMM_WORLD);
      grad = Map<MatrixXd>( who, grad.rows(), grad.cols() );
      grad = grad / (BATCH_SIZE * size);
      delete[] what;
      delete[] who;
    }
    network.update_add(std::move(gradients));
    if (myrank == root && i % LOG == 0) {
      result = network(batch_x);
      cost = (batch_y - result);
      after_cost = (cost.array().pow(2) / 2).mean();
      cout << "Cost Before: " << before_cost << " After: " << after_cost << endl;
    }
  }

  if(myrank == root) {
    MatrixXd result_argmax = argmax(result);
    MatrixXd true_argmax = argmax(batch_y);

    auto accuracy = (result_argmax.array() == true_argmax.array()).cast<float>();
    float acc = accuracy.mean();

    cout << "Accuracy: " << acc << endl;
    cout << "Time Elapsed: " << time(NULL) - begin << endl;
  }

  MPI_Finalize();
  return 0;
}