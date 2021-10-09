/*
 * MIT License
 *
 * Copyright (c) 2021 Dan Logan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */













#include <iostream>















#include "match.hpp"
#include <algorithm>
#include <QMessageBox>
#include <QLocale>

#define BYE_PLAYER_ID 10

constexpr const char* P_ONE_LBL = "player_one";
constexpr const char* P_TWO_LBL = "player_two";

void Match::setupTables()
{
    m_matchView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    m_matchView->setSortingEnabled(false);
    m_matchView->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("P1")));
    m_matchView->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("P2")));
    m_matchView->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("W1")));
    m_matchView->setHorizontalHeaderItem(3, new QTableWidgetItem(tr("W2")));
    m_matchView->setHorizontalHeaderItem(4, new QTableWidgetItem(tr("Tie")));

    m_matchView->resizeColumnsToContents();
}

nlohmann::json Match::toJson() const
{
    nlohmann::json j;

    for (const auto& m : m_matchups)
    {
        j.emplace_back(); // add an empty element onto the end of the JSON list

        auto& elem = j.back();
        elem[P_ONE_LBL] = m.p1->getName().toStdString();

        if (m.p2 != nullptr)
        {
            elem[P_TWO_LBL] = m.p2->getName().toStdString();
        }
    }

    return j;
}

void Match::setEnabled(bool enable)
{
    m_generateMatchB->setEnabled(enable);
    m_matchView->setEnabled(enable);
    m_generateMatchB->setVisible(enable);
    m_matchView->setVisible(enable);

    m_matchView->resizeColumnsToContents();
}

void Match::generateMatch(const QList<std::shared_ptr<Player>> &playerList, std::int32_t matchNum)
{
    m_matchups.clear();
    //generate pairings
    auto sourceList = playerList;
    decltype(sourceList) editedList;
    while (!sourceList.empty())
    {
        std::uniform_int_distribution<std::size_t> dis(0, sourceList.size() - 1);
        auto index = dis(m_reng);
        editedList.push_back(sourceList[index]);
        sourceList.removeAt(index);
    }
    if (matchNum == 0)
    {
        for (int i = 0; i < editedList.size(); i += 2)
        {
            if (editedList.size() >= i + 2)
                m_matchups.emplace_back(Matchup{editedList[i], editedList[i + 1]});
            else
                m_matchups.emplace_back(Matchup{editedList[i], nullptr});
        }
    }
    else
    {
        int b = 0;
        int m = 0;
        std::sort(editedList.begin(), editedList.end(), [match = (matchNum - 1)](std::shared_ptr<Player> p1, std::shared_ptr<Player> p2)
                  { return p1->getMatchScore(match) > p2->getMatchScore(match); }); //use > for reverse sort
        while (!generatePairing(editedList, matchNum - 1, b, m) && b <= matchNum && m <= matchNum)
        {
            m_matchups.clear();
            m++;
            if (m > matchNum)
            {
                m = 0;
                b++;
            }
        }
        if (b > matchNum && m > matchNum)
        {

            QLocale locale;
            QMessageBox dialog;
            dialog.setWindowTitle(tr("Match ") + locale.toString(matchNum) + tr(" generation error."));
            dialog.setText(tr("Could not generate pairings for match"));
            dialog.exec();
            return;
        }
    }

    updateMatchView();
}
bool Match::loadMatch(const nlohmann::json& j, const QList<std::shared_ptr<Player>>& players, std::size_t matchNum)
{
    if (!j.is_array())
    {
        return false;
    }

    for (const auto p : j)
    {
        if (!p.contains(P_ONE_LBL))
        {
            return false;
        }
        const auto p1Name = p[P_ONE_LBL].get<std::string>();
        const auto p2Name = p.contains(P_TWO_LBL) ? p[P_TWO_LBL].get<std::string>() : "";

        // find the player whose name matches
        std::shared_ptr<Player> p1 = nullptr;
        std::shared_ptr<Player> p2 = nullptr;
        for (const auto& player : players)
        {
            if (player->getName() == QString::fromStdString(p1Name))
            {
                p1 = player;
            }
            else if (player->getName() == QString::fromStdString(p2Name))
            {
                p2 = player;
            }
        }

        if (p1 == nullptr)
        {
            return false;
        }

        m_matchups.emplace_back(Matchup{p1, p2});
    }

    updateMatchView();
    updateMatchResultsView(matchNum);

    //set the output for the match results if applicable
    return true;
}

bool Match::finalizeMatch(const QList<std::shared_ptr<Player>> &playerList, std::int32_t matchNum)
{
    for (int i = 0; i < m_matchups.size(); i++)
    {
        MatchResult res;
        res.bye = m_matchups[i].p2 == nullptr;
        res.opponent = m_matchups[i].p2;
        res.wins = m_matchView->item(i, 2)->text().toInt();
        res.losses = m_matchView->item(i, 3)->text().toInt();
        res.ties = m_matchView->item(i, 4)->text().toInt();
        res.matchWin = res.wins > res.losses;
        res.matchTie = res.wins == res.losses;
        m_matchups[i].p1->setMatchResults(matchNum, res);

        if (!res.bye)
        {
            std::swap(res.wins, res.losses);
            res.opponent = m_matchups[i].p1;
            res.matchWin = !res.matchWin && !res.matchTie;
            m_matchups[i].p2->setMatchResults(matchNum, res);
        }
    }

    for (const auto &player : playerList)
    {
        QString reason;
        if (!player->scoresValid(&reason, matchNum))
        {
            QLocale locale;
            QMessageBox dialog;
            dialog.setWindowTitle(tr("Match ") + locale.toString(matchNum) + tr(" error."));
            dialog.setText(player->getName() + tr(": ") + reason);
            dialog.exec();
            return false;
        }
    }

    m_matchView->resizeColumnsToContents();
    return true;
}

bool Match::generatePairing(const QList<std::shared_ptr<Player>> &playerList, std::int32_t matchNum, std::int32_t maxByes, std::int32_t maxMatchups)
{
    if (playerList.empty())
        return false;
    auto pairList = playerList;
    auto p1 = pairList[0];
    pairList.removeFirst();
    for (const auto &p2 : pairList)
    {
        if (p1->getPreviousOpponents(matchNum).count(p2->getId()) > maxMatchups || p2->getPreviousOpponents(matchNum).count(p1->getId()) > maxMatchups) //already played too many times
            continue;
        if (pairList.size() == 1) //only one player left, and it's valid. return true
        {
            m_matchups.push_front(Matchup{p1, p2});
            return true;
        }
        auto remaining = pairList;
        remaining.removeOne(p2);
        if (generatePairing(remaining, matchNum, maxByes, maxMatchups))
        {
            m_matchups.push_front(Matchup{p1, p2});
            return true;
        }
    }
    if ((pairList.size() & 1) == 0) //if no pairs found for current player, and there is an even number of other players
    {
        if (p1->receivedByes(matchNum) > maxByes)
            return false;
        if (generatePairing(pairList, matchNum, maxByes, maxMatchups))
        {
            m_matchups.push_front(Matchup{p1, nullptr});
            return true;
        }
    }
    return false;
}

void Match::updateMatchView()
{
    m_matchView->setRowCount(m_matchups.size());

    //write pairings to table
    for (int i = 0; i < m_matchups.size(); i++)
    {
        if (m_matchups[i].p1 != nullptr) //should never fail, but...
        {
            auto item = new QTableWidgetItem(m_matchups[i].p1->getName());
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            m_matchView->setItem(i, 0, item);
        }
        if (m_matchups[i].p2 != nullptr)
        {
            auto item = new QTableWidgetItem(m_matchups[i].p2->getName());
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            m_matchView->setItem(i, 1, item);
            item = new QTableWidgetItem(tr(""));
            item->setFlags(item->flags() | Qt::ItemIsEditable);
            m_matchView->setItem(i, 2, item);
            item = new QTableWidgetItem(tr(""));
            item->setFlags(item->flags() | Qt::ItemIsEditable);
            m_matchView->setItem(i, 3, item);
            item = new QTableWidgetItem(tr(""));
            item->setFlags(item->flags() | Qt::ItemIsEditable);
            m_matchView->setItem(i, 4, item);
        }
        else
        {
            auto item = new QTableWidgetItem(tr("Bye"));
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            m_matchView->setItem(i, 1, item);
            //fill in the bye info
            item = new QTableWidgetItem(tr("2"));
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            m_matchView->setItem(i, 2, item);
            item = new QTableWidgetItem(tr("0"));
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            m_matchView->setItem(i, 3, item);
            item = new QTableWidgetItem(tr("0"));
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            m_matchView->setItem(i, 4, item);
        }
    }

    m_matchView->resizeColumnsToContents();
}
void Match::updateMatchResultsView(std::size_t matchNum)
{
    for (int i = 0; i < m_matchups.size(); i++)
    {
        const auto& p1 = m_matchups[i].p1;
        const auto& p2 = m_matchups[i].p2;

        if (p1 == nullptr)
        {
            std::cerr << "Couldn't update match results, no player 1";
            return;
        }
        m_matchView->item(i, 2)->setText(QString::number(p1->getMatchResult(matchNum).wins));
        m_matchView->item(i, 3)->setText(QString::number(p1->getMatchResult(matchNum).losses));
        m_matchView->item(i, 4)->setText(QString::number(p1->getMatchResult(matchNum).ties));
    }
}
