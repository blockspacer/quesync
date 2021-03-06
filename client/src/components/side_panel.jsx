import React, { Component } from "react";

import { connect } from "react-redux";

import { Elevation } from "@rmwc/elevation";
import { Typography } from "@rmwc/typography";
import { IconButton } from "@rmwc/icon-button";
import { Tooltip } from "@rmwc/tooltip";
import "@rmwc/tooltip/tooltip.css";

import anime from "animejs";
import { Transition } from "react-transition-group";

import CurrentCallDetails from "./current_call_details";
import Seperator from "./seperator";

import { leaveCall, setVoiceState } from "../actions/voiceActions";

import "./side_panel.scss";

class SidePanel extends Component {
	parseVoiceStates() {
		let userVoiceStates = [];

		// Add the user itself
		userVoiceStates.push({
			id: this.props.user.id,
			nickname: this.props.user.nickname,
			avatar: this.props.profiles[this.props.user.id].photo,
			calling: false,
			muted: this.props.muted,
			deafen: this.props.deafen
		});

		// For each user
		for (const userId in this.props.voiceStates) {
			// If connected or pending
			if (this.props.voiceStates[userId].state) {
				userVoiceStates.push({
					id: userId,
					nickname: this.props.profiles[userId].nickname,
					avatar: this.props.profiles[userId].photo,
					calling: this.props.voiceStates[userId].state === 2,
					muted: this.props.voiceStates[userId].muted,
					deafen: this.props.voiceStates[userId].deafen
				});
			}
		}

		return userVoiceStates;
	}

	render() {
		const enterTransition = content =>
			anime({
				targets: content,
				duration: 800,
				easing: "easeInOutCirc",
				minHeight: "50%",
				height: "50%",
				opacity: 1
			});

		const exitTransition = content =>
			anime({
				targets: content,
				duration: 800,
				easing: "easeInOutCirc",
				minHeight: "0",
				height: "0",
				opacity: 0
			});

		return (
			<Elevation z={10} className="quesync-side-panel">
				<div className="quesync-side-panel-details">
					{this.props.element}
					<Seperator style={{ opacity: this.props.inCall ? 1 : 0 }} />
					<Transition
						appear
						unmountOnExit
						in={this.props.inCall}
						timeout={800}
						onEnter={enterTransition}
						onExit={exitTransition}
					>
						<CurrentCallDetails
							style={{
								minHeight: 0,
								height: 0,
								opacity: 0,
								pointerEvents: this.props.inCall ? "" : "none"
							}}
							joinDate={this.props.callJoinDate}
							userVoiceStates={this.props.inCall ? this.parseVoiceStates() : []}
							userVoiceActivations={this.props.voiceActivations}
							leaveCall={() => this.props.dispatch(leaveCall())}
						/>
					</Transition>
				</div>
				<Elevation z={5} className="quesync-user">
					<img
						className="quesync-user-avatar"
						src={this.props.profiles[this.props.user.id].photo}
						alt={this.props.user.nickname}
					/>
					<div className="quesync-user-name">
						<Typography className="quesync-user-nickname" use="headline6">
							{this.props.user.nickname}
						</Typography>
						<Typography className="quesync-user-tag" use="body2">
							#{this.props.user.tag}
						</Typography>
					</div>
					<div className="quesync-user-actions">
						<Tooltip
							className="quesync-tooltip"
							content={this.props.muted ? "Unmute" : "Mute"}
							showArrow
						>
							<IconButton
								className={
									this.props.muted
										? "quesync-user-action-button quesync-user-action-button-disabled"
										: "quesync-user-action-button"
								}
								icon={this.props.muted ? "mic_off" : "mic"}
								onClick={() =>
									this.props.dispatch(
										setVoiceState(!this.props.muted, this.props.deafen)
									)
								}
							/>
						</Tooltip>
						<Tooltip
							className="quesync-tooltip"
							content={this.props.deafen ? "Undeafen" : "Deafen"}
							showArrow
						>
							<IconButton
								className={
									this.props.deafen
										? "quesync-user-action-button quesync-user-action-button-disabled"
										: "quesync-user-action-button"
								}
								icon={this.props.deafen ? "volume_off" : "volume_up"}
								onClick={() =>
									this.props.dispatch(
										setVoiceState(this.props.muted, !this.props.deafen)
									)
								}
							/>
						</Tooltip>
					</div>
				</Elevation>
			</Elevation>
		);
	}
}

export default connect(state => ({
	user: state.user.user,
	inCall: !!state.voice.channelId,
	callJoinDate: state.voice.callJoinDate,
	voiceStates: state.voice.voiceStates,
	voiceActivations: state.voice.voiceActivations,
	muted: state.voice.muted,
	deafen: state.voice.deafen,
	profiles: state.users.profiles
}))(SidePanel);
