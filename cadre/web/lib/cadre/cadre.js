/**
 * Script to manage the whole interface
 */

$(function() {

	var frameAddress = "192.168.0.1";

	/**
	 * Setup the page when starting
	 */
	function setupPage() {
		// Load the initial state
		loadConfig();
		loadState();
		// Bind events
		$('#mode').change(displayMode);
		$('#settings').click(displaySettings);
		$('#send').click(send);
		$('#cancel').click(cancel);
		// Show the initial state
		displayMode();
	}

	/**
	 * Load the current state from local storage
	 */
	function loadState() {
		if (localStorage.getItem("stateStored") == 1) {
			// The state is stored
			$('select, input[type!=button]').each(function() {
				if ($(this).attr('type') == 'color')
					$(this).spectrum("set", localStorage.getItem(this.id));
				else
					$(this).val(localStorage.getItem(this.id));
			});
		}
	}

	/**
	 * Save the current state to local storage
	 */
	function saveState() {
		// Set input states
		localStorage.setItem("stateStored", 1);
		$('select, input[type!=button]').each(function() {
			localStorage.setItem(this.id, $(this).val());
		});
	}

	/**
	 * Load the current config from local storage
	 */
	function loadConfig() {
		if (localStorage.getItem("frameAddress") != null) {
			// The state is stored
			frameAddress = localStorage.getItem("frameAddress");
		}
	}

	/**
	 * Save the current config to local storage
	 */
	function saveConfig() {
		// Set settings
		localStorage.setItem("frameAddress", frameAddress);
	}

	/**
	 * Display the selected mode
	 */
	function displayMode() {
		var mode = $('#mode').val();
		$(".config").hide();
		$("#" + mode).show();
	}

	/**
	 * Display the settings modal window
	 */
	function displaySettings() {
		var address = prompt("Adresse du cadre :", frameAddress);
		if (address) {
			frameAddress = address;
			saveConfig();
		}
	}

	/**
	 * Send the value to the frame
	 */
	function send() {
		$('.action').attr('disabled', true);
		// Prepare data to send
		var settingsBlock = $('#' + $('#mode').val());
		// Add the mode
		var toSend = settingsBlock.attr('data-id');
		while (toSend.length < 2)
			toSend = "0" + toSend;
		// Add the value of each settings element
		var counter = 0;
		var settingEl = settingsBlock.find('[data-send=' + counter + ']');
		while (settingEl.length > 0) {
			if (settingEl.attr('type') == 'color') {
				// The setting is a color, send the three color components
				var color = settingEl.spectrum('get').toRgb();
				$(['r','g','b']).each(function() {
					var string = '' + color[this];
					while (string.length < 3)
						string = '0' + string;
					toSend+= string;
				});
			} else if (settingEl.attr('type') == 'time') {
				// The setting is a time, send the hours and minutes
				var time = settingEl.val().split(':');
				var string = time[1].trim();
				while (string.length < 2)
					string = '0' + string;
				string = time[0].trim() + string;
				while (string.length < 4)
					string = '0' + string;
				toSend+= string;
			} else if (settingEl.attr('type') == 'range') {
				// The setting is a number in a range
				var length = ('' + settingEl.prop('max')).length;
				var value = settingEl.val();
				while (value.length < length)
					value = '0' + value;
				toSend+= value;
			} else if (settingEl.attr('data-auto') == 'timestamp') {
				// Automatic timestamp
				toSend+= Math.floor(Date.now() / 1000 - (new Date()).getTimezoneOffset() * 60);
			} else {
				// Default setting type: send directly the value
				toSend+= settingEl.val();
			}
			counter++;
			settingEl = settingsBlock.find('[data-send=' + counter + ']');
		}
		// Send data
		$.post("http://" + frameAddress + "/setMode?" + toSend, function() {
			// Data successfuly sent
			$('.action').attr('disabled', false);
			// Save locally the current state
			saveState();
		}).fail(function() {
			// Failed sending data
			$('.action').attr('disabled', false);
			alert("Le cadre n'a pas pu être contacté à l'adresse " + frameAddress + ".");
		});
	}

	// Display the locally stored state
	function cancel() {
		loadState();
		displayMode();
	}

	setupPage();
});