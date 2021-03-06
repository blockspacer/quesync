import React, { Component } from "react";

import anime from "animejs";

import { ThemeProvider } from "@rmwc/theme";
import { Elevation } from "@rmwc/elevation";
import { Typography } from "@rmwc/typography";
import { Button } from "@rmwc/button";

import "./call_window.scss";

const { ipcRenderer } = window.require("electron");

class CallWindow extends Component {
	componentDidMount() {
		// Set the window to auto-close itself after 20 seconds
		setTimeout(() => {
			ipcRenderer.send("close-call-window", this.props.id);
		}, 20 * 1000);

		anime({
			targets: ".quesync-call-avatar",
			easing: "spring(0.2, 100, 4, 0)",
			loop: true,
			direction: "alternate",
			height: ["70px", "110px"],
			width: ["70px", "110px"],
			borderRadius: ["35px", "55px"],
			boxShadow: [
				"0px 0px 0px 0px #40c4ff",
				"0px 0px 4px 4px #40c4ff",
				"0px 0px 4.5px 4.5px #40c4ff",
				"0px 0px 5px 5px #40c4ff"
			]
		});
	}

	render() {
		return (
			<ThemeProvider
				className="quesync-call-window"
				options={{
					primary: "#00b0ff",
					secondary: "#282828",
					error: "#ff1744",
					onSurface: "rgba(255,255,255,.87)"
				}}>
				<Elevation className="quesync-call-window-content" z={6}>
					<div className="quesync-call-avatar-wrapper">
						<img
							className="quesync-call-avatar"
							src={this.props.avatar}
							alt="Incoming Call"
						/>
					</div>
					<Typography className="quesync-call-nickname" use="headline5">
						{this.props.nickname}
					</Typography>
					<div className="quesync-call-actions">
						<Button
							raised
							className="quesync-call-action-button quesync-call-accept-button"
							label="Answer"
							onClick={() => {
								ipcRenderer.send("call-main-window-event", {
									name: "join-call",
									arg: this.props.id
								});
							}}
						/>
						<Button
							raised
							danger
							className="quesync-call-action-button quesync-call-decline-button"
							label="Reject"
							onClick={() => {
								ipcRenderer.send("close-call-window", this.props.id);
							}}
						/>
					</div>
				</Elevation>
			</ThemeProvider>
		);
	}
}

export default CallWindow;
