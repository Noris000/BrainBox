#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QFont>
#include <QString>
#include <QSet>

#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

struct Question {
    std::string text;
    std::vector<std::string> options;
    std::vector<int> correctAnswers;   // 1-based
    bool isMultiple = false;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("BrainBox");
    window.resize(560, 520);

    QVBoxLayout *mainLayout = new QVBoxLayout(&window);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    QStackedWidget *screenStack = new QStackedWidget;
    QWidget *welcomeScreen = new QWidget;
    QWidget *quizScreen = new QWidget;
    QWidget *resultScreen = new QWidget;
    QVBoxLayout *welcomeLayout = new QVBoxLayout(welcomeScreen);
    QVBoxLayout *quizLayout = new QVBoxLayout(quizScreen);
    QVBoxLayout *resultLayout = new QVBoxLayout(resultScreen);
    welcomeLayout->setSpacing(10);
    quizLayout->setSpacing(10);
    resultLayout->setSpacing(10);
    welcomeLayout->setContentsMargins(0, 0, 0, 0);
    quizLayout->setContentsMargins(0, 0, 0, 0);
    resultLayout->setContentsMargins(0, 0, 0, 0);

    // ========================================================
    // Welcome screen
    // ========================================================
    QLabel *titleLabel = new QLabel("Welcome to BrainBox");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(22);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel *messageLabel = new QLabel("Choose the type of quiz:");
    QFont messageFont = messageLabel->font();
    messageFont.setPointSize(16);
    messageLabel->setFont(messageFont);
    messageLabel->setAlignment(Qt::AlignCenter);

    QPushButton *btnSingle = new QPushButton("Single Answer Questions");
    QPushButton *btnMulti  = new QPushButton("Multiple Answer Questions");
    QFont choiceFont = btnSingle->font();
    choiceFont.setPointSize(13);
    btnSingle->setFont(choiceFont);
    btnMulti->setFont(choiceFont);
    btnSingle->setMinimumHeight(50);
    btnMulti->setMinimumHeight(50);

    welcomeLayout->addStretch();
    welcomeLayout->addWidget(titleLabel);
    welcomeLayout->addSpacing(8);
    welcomeLayout->addWidget(messageLabel);
    welcomeLayout->addSpacing(12);
    welcomeLayout->addWidget(btnSingle);
    welcomeLayout->addWidget(btnMulti);
    welcomeLayout->addStretch();

    // ========================================================
    // Quiz widgets
    // ========================================================
    QLabel *questionLabel = new QLabel;
    questionLabel->setWordWrap(true);
    questionLabel->setAlignment(Qt::AlignCenter);
    QFont qFont = questionLabel->font();
    qFont.setPointSize(13);
    questionLabel->setFont(qFont);
    questionLabel->hide();

    QWidget *optionsContainer = new QWidget;
    QVBoxLayout *optionsLayout = new QVBoxLayout(optionsContainer);
    optionsLayout->setContentsMargins(0, 0, 0, 0);
    optionsLayout->setSpacing(6);
    optionsContainer->hide();

    QLabel *feedbackLabel = new QLabel;
    feedbackLabel->setAlignment(Qt::AlignCenter);
    feedbackLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    feedbackLabel->hide();

    QPushButton *checkButton = new QPushButton("Check Answer");
    checkButton->setMinimumHeight(40);
    checkButton->hide();

    QPushButton *nextButton = new QPushButton("Next Question →");
    nextButton->setMinimumHeight(40);
    nextButton->hide();

    QLabel *resultLabel = new QLabel;
    resultLabel->setAlignment(Qt::AlignCenter);
    resultLabel->setWordWrap(true);
    QFont resultFont = resultLabel->font();
    resultFont.setPointSize(16);
    resultLabel->setFont(resultFont);

    QPushButton *backButton = new QPushButton("Back to Start");
    QFont resultButtonFont = backButton->font();
    resultButtonFont.setPointSize(12);
    backButton->setFont(resultButtonFont);
    backButton->setMinimumHeight(50);

    quizLayout->addWidget(questionLabel);
    quizLayout->addWidget(optionsContainer);
    quizLayout->addWidget(feedbackLabel);
    quizLayout->addWidget(checkButton, 0, Qt::AlignCenter);
    quizLayout->addWidget(nextButton, 0, Qt::AlignCenter);
    resultLayout->addStretch();
    resultLayout->addWidget(resultLabel);
    resultLayout->addWidget(backButton, 0, Qt::AlignCenter);
    resultLayout->addStretch();

    screenStack->addWidget(welcomeScreen);
    screenStack->addWidget(quizScreen);
    screenStack->addWidget(resultScreen);
    mainLayout->addWidget(screenStack);

    // ========================================================
    // State
    // ========================================================
    std::vector<Question> quizQuestions;
    std::vector<QPushButton*> optionButtons;
    int currentIndex = 0;
    int score = 0;
    int maxPossibleScore = 0;
    bool isMultipleMode = false;

    // ========================================================
    // Load questions from JSON
    // ========================================================
    auto loadQuestions = [&](const std::string& filename, bool multiple) -> bool {
        std::ifstream file(filename);
        if (!file.is_open()) {
            QMessageBox::critical(nullptr, "Error",
                QString("Could not open %1\nMake sure the file is next to the program.")
                    .arg(QString::fromStdString(filename)));
            return false;
        }

        json data;
        try {
            file >> data;
        } catch (const json::parse_error& e) {
            QMessageBox::critical(nullptr, "Error",
                QString("JSON error:\n%1").arg(e.what()));
            return false;
        }
        file.close();

        if (!data.contains("questions") || !data["questions"].is_array()) {
            QMessageBox::critical(nullptr, "Error", "No questions found in the file.");
            return false;
        }

        quizQuestions.clear();
        maxPossibleScore = 0;

        // JSON answer numbers and the labels shown to the user are both 1-based.
        for (const auto& q : data["questions"]) {
            Question question;
            question.text = q["question"].get<std::string>();
            question.isMultiple = multiple;

            for (const auto& opt : q["options"]) {
                question.options.push_back(opt.get<std::string>());
            }

            if (multiple) {
                for (const auto& a : q["answers"]) {
                    question.correctAnswers.push_back(a.get<int>());
                }
            } else {
                question.correctAnswers.push_back(q["answer"].get<int>());
            }

            maxPossibleScore += static_cast<int>(question.correctAnswers.size());
            quizQuestions.push_back(question);
        }

        // Randomize the order for each quiz attempt without changing answer indices.
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(quizQuestions.begin(), quizQuestions.end(), g);

        return true;
    };

    // ========================================================
    // Clear old option buttons
    // ========================================================
    auto clearOptionButtons = [&]() {
        for (QPushButton* btn : optionButtons) {
            optionsLayout->removeWidget(btn);
            btn->deleteLater();
        }
        optionButtons.clear();
    };

    // ========================================================
    // Show current question
    // ========================================================
    auto showCurrentQuestion = [&]() {
        clearOptionButtons();

        const Question& q = quizQuestions[currentIndex];
        const int numCorrect = static_cast<int>(q.correctAnswers.size());

        QString header = QString("<b>Question %1 of %2</b><br><br>%3")
            .arg(currentIndex + 1)
            .arg(quizQuestions.size())
            .arg(QString::fromStdString(q.text));

        if (q.isMultiple) {
            header += QString("<br><br><i>Select up to %1 correct answer(s)</i>").arg(numCorrect);
        }

        questionLabel->setText(header);
        questionLabel->show();
        optionsContainer->show();

        // Build choices from the JSON so questions are not limited to a fixed option count.
        for (size_t i = 0; i < q.options.size(); ++i) {
            QPushButton *btn = new QPushButton(
                QString("%1.  %2")
                    .arg(i + 1)
                    .arg(QString::fromStdString(q.options[i]))
            );
            btn->setMinimumHeight(36);
            btn->setCheckable(true);
            btn->setEnabled(true);

            optionsLayout->addWidget(btn);
            optionButtons.push_back(btn);

            // In single-answer mode, selecting one choice submits it immediately.
            if (!q.isMultiple) {
                QObject::connect(btn, &QPushButton::clicked, [&, i]() {
                    for (QPushButton* b : optionButtons)
                        b->setEnabled(false);

                    const Question& qq = quizQuestions[currentIndex];
                    int chosen = static_cast<int>(i + 1);

                    if (chosen == qq.correctAnswers[0]) {
                        feedbackLabel->setText("Correct! +1 point");
                        feedbackLabel->setStyleSheet("color: green; font-weight: bold; font-size: 14px;");
                        score += 1;
                    } else {
                        QString correctText = QString::fromStdString(
                            qq.options[qq.correctAnswers[0] - 1]);
                        feedbackLabel->setText(
                            QString("Wrong. Correct answer was %1. %2")
                                .arg(qq.correctAnswers[0])
                                .arg(correctText)
                        );
                        feedbackLabel->setStyleSheet("color: red; font-weight: bold; font-size: 14px;");
                    }

                    feedbackLabel->show();
                    nextButton->show();
                });
            }
        }

        feedbackLabel->hide();
        nextButton->hide();

        if (q.isMultiple) {
            checkButton->show();
        } else {
            checkButton->hide();
        }
    };

    // ========================================================
    // Start quiz
    // ========================================================
    auto startQuiz = [&](bool multiple) {
        isMultipleMode = multiple;
        std::string filename = multiple ? "questions_multi.json" : "questions_single.json";

        if (!loadQuestions(filename, multiple))
            return;

        screenStack->setCurrentWidget(quizScreen);

        currentIndex = 0;
        score = 0;
        showCurrentQuestion();
    };

    QObject::connect(btnSingle, &QPushButton::clicked, [&]() { startQuiz(false); });
    QObject::connect(btnMulti,  &QPushButton::clicked, [&]() { startQuiz(true);  });

    QObject::connect(backButton, &QPushButton::clicked, [&]() {
        screenStack->setCurrentWidget(welcomeScreen);

        score = 0;
        currentIndex = 0;
    });

    // ========================================================
    // Check Answer (Multiple mode)
    // ========================================================
    QObject::connect(checkButton, &QPushButton::clicked, [&]() {
        const Question& q = quizQuestions[currentIndex];
        const int numCorrect = static_cast<int>(q.correctAnswers.size());
        QString correctAnswersText;
        for (int answer : q.correctAnswers) {
            if (!correctAnswersText.isEmpty())
                correctAnswersText += ", ";
            correctAnswersText += QString("%1. %2")
                .arg(answer)
                .arg(QString::fromStdString(q.options[answer - 1]));
        }

        // Sets make membership checks simple and prevent duplicate answer indices.
        QSet<int> selected;
        for (size_t i = 0; i < optionButtons.size(); ++i) {
            if (optionButtons[i]->isChecked()) {
                selected.insert(static_cast<int>(i + 1));
            }
        }

        for (QPushButton* b : optionButtons)
            b->setEnabled(false);

        checkButton->hide();

        // Selecting more choices than the question has correct answers earns no points.
        if (selected.size() > static_cast<size_t>(numCorrect)) {
            feedbackLabel->setText(
                QString("Too many answers selected (max %1). 0 points.\nCorrect answers: %2")
                    .arg(numCorrect)
                    .arg(correctAnswersText)
            );
            feedbackLabel->setStyleSheet("color: red; font-weight: bold; font-size: 14px;");
            feedbackLabel->show();
            nextButton->show();
            return;
        }

        QSet<int> correctSet;
        for (int a : q.correctAnswers)
            correctSet.insert(a);

        int pointsThisQuestion = 0;
        // Award one point for each selected correct choice; incorrect choices do not subtract points.
        for (int chosen : selected) {
            if (correctSet.contains(chosen)) {
                pointsThisQuestion++;
            }
        }

        score += pointsThisQuestion;

        if (pointsThisQuestion == numCorrect && selected.size() == static_cast<size_t>(numCorrect)) {
            feedbackLabel->setText(
                QString("Perfect! +%1 point(s)\nCorrect answers: %2")
                    .arg(pointsThisQuestion)
                    .arg(correctAnswersText)
            );
            feedbackLabel->setStyleSheet("color: green; font-weight: bold; font-size: 14px;");
        } else if (pointsThisQuestion > 0) {
            feedbackLabel->setText(
                QString("Partial credit: +%1 point(s)\nCorrect answers: %2")
                    .arg(pointsThisQuestion)
                    .arg(correctAnswersText)
            );
            feedbackLabel->setStyleSheet("color: orange; font-weight: bold; font-size: 14px;");
        } else {
            feedbackLabel->setText(
                QString("Wrong. 0 points.\nCorrect answers: %1").arg(correctAnswersText)
            );
            feedbackLabel->setStyleSheet("color: red; font-weight: bold; font-size: 14px;");
        }

        feedbackLabel->show();
        nextButton->show();
    });

    // ========================================================
    // Next Question + Results + Back to Start
    // ========================================================
    QObject::connect(nextButton, &QPushButton::clicked, [&]() {
        currentIndex++;

        if (currentIndex >= static_cast<int>(quizQuestions.size())) {
            // Hide quiz widgets
            questionLabel->hide();
            optionsContainer->hide();
            feedbackLabel->hide();
            checkButton->hide();
            nextButton->hide();
            clearOptionButtons();

            double percentage = 0.0;
            if (maxPossibleScore > 0) {
                // Multiple-answer questions can be worth several points, so use total correct choices.
                percentage = (static_cast<double>(score) / maxPossibleScore) * 100.0;
            }

            QString resultText = QString(
                "<h2>Quiz Finished!</h2>"
                "You scored <b>%1</b> out of <b>%2</b> possible points<br>"
                "Percentage: <b>%3%</b>"
            ).arg(score)
             .arg(maxPossibleScore)
             .arg(percentage, 0, 'f', 1);

            resultLabel->setText(resultText);
            screenStack->setCurrentWidget(resultScreen);
        } else {
            showCurrentQuestion();
        }
    });

    window.show();
    return app.exec();
}