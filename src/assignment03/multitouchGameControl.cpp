#include "multitouchGameControl.hpp"
#include <iostream>

using namespace std;

MultitouchGameControl::MultitouchGameControl(Game& game, HomograpyCalibrator& calibrator) : m_game(game), m_calibrator(calibrator)
{
}

void MultitouchGameControl::handleInput(std::vector<cv::RotatedRect> positions)
{
    /*~~~~~~~~~~~*
     * YOUR CODE *
     * GOES HERE *
     *~~~~~~~~~~~*/

	for (Player& player : m_game.players()) {
		for (Paddle& paddle : player.paddles()) {
			paddle.setActive(false);
		}
	}

	for (cv::RotatedRect& position : positions) {
		cv::Point2f pos = m_calibrator.cameraToGame(position.center);
		float canvas_center = m_game.gameCanvasSize().width / 2.0f;
		Player& player = pos.x < canvas_center ? m_game.players()[0] : m_game.players()[1];
		
		// We take the distance from the left of each screen half
		// E.g. for the right player from the middle of the screen to pos or the paddles
		float xPosSide = fmod(pos.x, canvas_center);
		double paddle0Dist = fmod(player.paddles()[0].center(), canvas_center);
		double paddle1Dist = fmod(player.paddles()[1].center(), canvas_center);

		Paddle& leftPaddle = paddle0Dist <= paddle1Dist ? player.paddles()[0] : player.paddles()[1];
		Paddle& rightPaddle = paddle0Dist > paddle1Dist ? player.paddles()[0] : player.paddles()[1];

		Paddle& paddle = xPosSide < fmod(rightPaddle.center(), canvas_center) ? leftPaddle : rightPaddle;

		// check to make sure we are not activating the same padle twice
		if (!paddle.active()) {
			paddle.setActive(true);
			paddle.setTargetPosition(pos.y);
		}
	}
    // Use m_calibrator to access and modify the m_game's player's pedals.
    // How you do it is your call, but you should probably use active(), setActive(), position() and setTargetPosition().
}
